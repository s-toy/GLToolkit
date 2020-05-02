#include "Model.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>
#include "Texture.h"
#include "Common.h"
#include "FileLocator.h"
#include "Utility.h"

using namespace glt;

std::unordered_map<std::string, CModel*> CModel::m_ExsitedModelMap;

//***********************************************************************************************
//FUNCTION:
CModel::CModel(const std::string& vFilePath)
{
	m_pImporter = std::make_shared<Assimp::Importer>();
	m_pBoneInfo = std::make_shared<std::vector<SBoneInfo>>();
	if (!__loadModel(vFilePath))
	{
		_OUTPUT_WARNING(format("Failed to load model at %s.", vFilePath.c_str()));
		_ASSERTE(false);
	}
}

//***********************************************************************************************
//FUNCTION:
CModel::~CModel()
{
}

//**********************************************************************************************
//FUNCTION:
bool CModel::__loadModel(const std::string& vPath)
{
	std::string FilePath = CFileLocator::getInstance()->locateFile(vPath);
	_ASSERTE(!FilePath.empty());

	auto iter = m_ExsitedModelMap.find(FilePath);
	if (iter != m_ExsitedModelMap.end())
	{
		if (nullptr == m_ExsitedModelMap[FilePath]) m_ExsitedModelMap.erase(iter);
		else { *this = *m_ExsitedModelMap[FilePath]; return true; }
	}

	m_pScene = m_pImporter->ReadFile(FilePath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals);

	if (!m_pScene || m_pScene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !m_pScene->mRootNode) { return false; }
	m_GlobalInverseTransform = aiMatrix4x4ToGlm(&(m_pScene->mRootNode->mTransformation));
	glm::inverse(m_GlobalInverseTransform);

	m_Directory = FilePath.substr(0, FilePath.find_last_of('/'));
	__processNode(m_pScene->mRootNode);

	m_ExsitedModelMap.insert(std::make_pair(FilePath, this));

	return true;
}

//**********************************************************************************************
//FUNCTION:
void CModel::__processNode(const aiNode* vNode)
{
	for (GLuint i = 0; i < vNode->mNumMeshes; ++i)
	{
		aiMesh* mesh = m_pScene->mMeshes[vNode->mMeshes[i]];
		m_Meshes.push_back(__processMesh(mesh));
	}

	for (GLuint i = 0; i < vNode->mNumChildren; ++i)
	{
		__processNode(vNode->mChildren[i]);
	}
}

//**********************************************************************************************
//FUNCTION:
std::shared_ptr<CMesh> CModel::__processMesh(const aiMesh* vMesh)
{
	std::vector<SVertex> Vertices(vMesh->mNumVertices);
	std::vector<unsigned> Indices(3u * vMesh->mNumFaces);
	std::vector<std::shared_ptr<CTexture2D>> Textures;

	_ASSERTE(vMesh->HasNormals());
	for (unsigned i = 0; i < vMesh->mNumVertices; ++i)
	{
		aiVector3D* pPosition = &(vMesh->mVertices[i]);
		aiVector3D* pNormal = &(vMesh->mNormals[i]);
		aiVector3D* pTexCoords = vMesh->mTextureCoords[0];

		Vertices[i].Position = glm::vec3(pPosition->x, pPosition->y, pPosition->z);
		Vertices[i].Normal = glm::vec3(pNormal->x, pNormal->y, pNormal->z);
		Vertices[i].TexCoords = pTexCoords ? glm::vec2(pTexCoords[i].x, pTexCoords[i].y) : glm::vec2(0.0f);
	}

	if (vMesh->HasBones()) m_HasBones = true; //TODO:不一定所有mesh都有bones
	for (unsigned i = 0; i < vMesh->mNumBones; ++i) 
	{
		unsigned BoneIndex = 0;
		std::string BoneName(vMesh->mBones[i]->mName.data);

		if (m_BoneName2IndexMap.find(BoneName) == m_BoneName2IndexMap.end())
		{
			BoneIndex = m_NumBones;
			m_NumBones++;
			m_pBoneInfo->push_back(SBoneInfo());
			(*m_pBoneInfo)[BoneIndex].BoneOffset = aiMatrix4x4ToGlm(&(vMesh->mBones[i]->mOffsetMatrix));
			m_BoneName2IndexMap[BoneName] = BoneIndex;
		}
		else
		{
			BoneIndex = m_BoneName2IndexMap[BoneName];
		}

		for (unsigned k = 0; k < vMesh->mBones[i]->mNumWeights; k++)
		{
			unsigned VertexID = vMesh->mBones[i]->mWeights[k].mVertexId;
			_ASSERTE(VertexID < Vertices.size());

			for (int m = 0; m < 4; ++m)
			{
				if (Vertices[VertexID].BoneWeights[m] == 0.0)
				{
					Vertices[VertexID].BoneIDs[m] = BoneIndex;
					Vertices[VertexID].BoneWeights[m] = vMesh->mBones[i]->mWeights[k].mWeight;
					break;
				}
			}
		}
	}

	for (unsigned i = 0; i < vMesh->mNumFaces; ++i)
	{
		aiFace Face = vMesh->mFaces[i];
		_ASSERTE(Face.mNumIndices == 3);
		Indices.push_back(Face.mIndices[0]);
		Indices.push_back(Face.mIndices[1]);
		Indices.push_back(Face.mIndices[2]);
	}

	if (vMesh->mMaterialIndex >= 0)
	{
		aiMaterial* pMaterial = m_pScene->mMaterials[vMesh->mMaterialIndex];

		std::vector<std::shared_ptr<CTexture2D>> DiffuseMaps = __loadMaterialTextures(pMaterial, aiTextureType_DIFFUSE, "uMaterialDiffuse");
		Textures.insert(Textures.end(), DiffuseMaps.begin(), DiffuseMaps.end());

		std::vector<std::shared_ptr<CTexture2D>> SpecularMaps = __loadMaterialTextures(pMaterial, aiTextureType_SPECULAR, "uMaterialSpecular");
		Textures.insert(Textures.end(), SpecularMaps.begin(), SpecularMaps.end());
	}

	return std::make_shared<CMesh>(Vertices, Indices, Textures);
}

//**********************************************************************************************
//FUNCTION:
std::vector<std::shared_ptr<CTexture2D>> CModel::__loadMaterialTextures(const aiMaterial* vMat, aiTextureType vType, const std::string& vTypeName)
{
	std::vector<std::shared_ptr<CTexture2D>> Textures;
	for (GLuint i = 0; i < vMat->GetTextureCount(vType); ++i)
	{
		aiString Str;
		vMat->GetTexture(vType, i, &Str);

		GLboolean Skip = false;
		for (GLuint j = 0; j < m_LoadedTextures.size(); j++)
		{
			if (std::strcmp(m_LoadedTextures[j]->getFilePath().c_str(), Str.C_Str()) == 0)
			{
				Textures.push_back(m_LoadedTextures[j]);
				Skip = true;
				break;
			}
		}
		if (!Skip)
		{
			auto TexturePath = this->m_Directory + std::string("/") + std::string(Str.C_Str());

			std::shared_ptr<CTexture2D> pTempTexture = std::make_shared<CTexture2D>();
			pTempTexture->load(TexturePath.c_str(), GL_REPEAT, GL_LINEAR);
			pTempTexture->setTextureName(vTypeName);
			Textures.push_back(pTempTexture);
			this->m_LoadedTextures.push_back(pTempTexture);
		}
	}
	return Textures;
}

//***********************************************************************************************
//FUNCTION:
void CModel::_draw(const CShaderProgram& vShaderProgram) const
{
	for (const auto& Mesh : m_Meshes)
	{
		Mesh->_draw(vShaderProgram);
	}
}

//***********************************************************************************************
//FUNCTION:
void CModel::_boneTransform(float vTimeInSeconds, std::vector<glm::mat4>& voTransforms) const
{
	glm::mat4 Identity = glm::identity<glm::mat4>();
	_ASSERTE(m_pScene->HasAnimations());

	float TicksPerSecond = m_pScene->mAnimations[0]->mTicksPerSecond != 0 ? m_pScene->mAnimations[0]->mTicksPerSecond : 25.0f;
	float TimeInTicks = vTimeInSeconds * TicksPerSecond;
	float AnimationTime = std::fmod(TimeInTicks, m_pScene->mAnimations[0]->mDuration);

	__readNodeHeirarchy(AnimationTime, m_pScene->mRootNode, Identity);

	voTransforms.resize(m_NumBones);
	for (unsigned i = 0; i < m_NumBones; i++) voTransforms[i] = (*m_pBoneInfo)[i].FinalTransformation;
}

//***********************************************************************************************
//FUNCTION:
void CModel::__readNodeHeirarchy(float vAnimationTime, const aiNode* vNode, const glm::mat4& vParentTransform) const
{
	std::string NodeName(vNode->mName.data);

	const aiAnimation* pAnimation = m_pScene->mAnimations[0];

	glm::mat4 NodeTransformation = aiMatrix4x4ToGlm(&(vNode->mTransformation));

	const aiNodeAnim* pNodeAnim = __findNodeAnim(pAnimation, NodeName);

	if (pNodeAnim)
	{
		// Interpolate scaling and generate scaling transformation matrix
		aiVector3D Scaling;
		__calcInterpolatedScaling(Scaling, vAnimationTime, pNodeAnim);
		glm::mat4 ScalingM = glm::scale(glm::mat4(1.0), glm::vec3(Scaling.x, Scaling.y, Scaling.z));

		// Interpolate rotation and generate rotation transformation matrix
		aiQuaternion RotationQ;
		__calcInterpolatedRotation(RotationQ, vAnimationTime, pNodeAnim);
		glm::mat4 RotationM = aiMatrix4x4ToGlm(&aiMatrix4x4(RotationQ.GetMatrix()));

		// Interpolate translation and generate translation transformation matrix
		aiVector3D Translation;
		__calcInterpolatedPosition(Translation, vAnimationTime, pNodeAnim);
		glm::mat4 TranslationM = glm::translate(glm::mat4(1.0), glm::vec3(Translation.x, Translation.y, Translation.z));

		// Combine the above transformations
		NodeTransformation = TranslationM * RotationM * ScalingM;
	}

	glm::mat4 GlobalTransformation = vParentTransform * NodeTransformation;

	if (m_BoneName2IndexMap.find(NodeName) != m_BoneName2IndexMap.end())
	{
		unsigned BoneIndex = m_BoneName2IndexMap.at(NodeName);
		(*m_pBoneInfo)[BoneIndex].FinalTransformation = m_GlobalInverseTransform * GlobalTransformation * (*m_pBoneInfo)[BoneIndex].BoneOffset;
	}

	for (unsigned i = 0; i < vNode->mNumChildren; ++i)
		__readNodeHeirarchy(vAnimationTime, vNode->mChildren[i], GlobalTransformation);
}

//***********************************************************************************************
//FUNCTION:
const aiNodeAnim* CModel::__findNodeAnim(const aiAnimation* vAnimation, const std::string vNodeName) const
{
	for (unsigned i = 0; i < vAnimation->mNumChannels; i++)
	{
		const aiNodeAnim* pNodeAnim = vAnimation->mChannels[i];
		if (std::string(pNodeAnim->mNodeName.data) == vNodeName) return pNodeAnim;
	}
	return nullptr;
}

//***********************************************************************************************
//FUNCTION:
void CModel::__calcInterpolatedPosition(aiVector3D& voVector, float vAnimationTime, const aiNodeAnim* vNodeAnim) const
{
	if (vNodeAnim->mNumPositionKeys == 1)
	{
		voVector = vNodeAnim->mPositionKeys[0].mValue;
		return;
	}

	unsigned PositionIndex = __findPosition(vAnimationTime, vNodeAnim);
	unsigned NextPositionIndex = (PositionIndex + 1);
	_ASSERTE(NextPositionIndex < vNodeAnim->mNumPositionKeys);
	float DeltaTime = (float)(vNodeAnim->mPositionKeys[NextPositionIndex].mTime - vNodeAnim->mPositionKeys[PositionIndex].mTime);
	float Factor = (vAnimationTime - (float)vNodeAnim->mPositionKeys[PositionIndex].mTime) / DeltaTime;
	_ASSERTE(Factor >= 0.0f && Factor <= 1.0f);
	const aiVector3D& Start = vNodeAnim->mPositionKeys[PositionIndex].mValue;
	const aiVector3D& End = vNodeAnim->mPositionKeys[NextPositionIndex].mValue;
	aiVector3D Delta = End - Start;
	voVector = Start + Factor * Delta;
}

//***********************************************************************************************
//FUNCTION:
void CModel::__calcInterpolatedScaling(aiVector3D& voVector, float vAnimationTime, const aiNodeAnim* vNodeAnim) const
{
	if (vNodeAnim->mNumScalingKeys == 1)
	{
		voVector = vNodeAnim->mScalingKeys[0].mValue;
		return;
	}

	unsigned ScalingIndex = __findScaling(vAnimationTime, vNodeAnim);
	unsigned NextScalingIndex = (ScalingIndex + 1);
	_ASSERTE(NextScalingIndex < vNodeAnim->mNumScalingKeys);
	float DeltaTime = (float)(vNodeAnim->mScalingKeys[NextScalingIndex].mTime - vNodeAnim->mScalingKeys[ScalingIndex].mTime);
	float Factor = (vAnimationTime - (float)vNodeAnim->mScalingKeys[ScalingIndex].mTime) / DeltaTime;
	_ASSERTE(Factor >= 0.0f && Factor <= 1.0f);
	const aiVector3D& Start = vNodeAnim->mScalingKeys[ScalingIndex].mValue;
	const aiVector3D& End = vNodeAnim->mScalingKeys[NextScalingIndex].mValue;
	aiVector3D Delta = End - Start;
	voVector = Start + Factor * Delta;
}

//***********************************************************************************************
//FUNCTION:
void CModel::__calcInterpolatedRotation(aiQuaternion& voQuaternion, float vAnimationTime, const aiNodeAnim* vNodeAnim) const
{
	// we need at least two values to interpolate...
	if (vNodeAnim->mNumRotationKeys == 1)
	{
		voQuaternion = vNodeAnim->mRotationKeys[0].mValue;
		return;
	}

	unsigned RotationIndex = __findRotation(vAnimationTime, vNodeAnim);
	unsigned NextRotationIndex = (RotationIndex + 1);
	_ASSERTE(NextRotationIndex < vNodeAnim->mNumRotationKeys);
	float DeltaTime = vNodeAnim->mRotationKeys[NextRotationIndex].mTime - vNodeAnim->mRotationKeys[RotationIndex].mTime;
	float Factor = (vAnimationTime - (float)vNodeAnim->mRotationKeys[RotationIndex].mTime) / DeltaTime;
	_ASSERTE(Factor >= 0.0f && Factor <= 1.0f);
	const aiQuaternion& StartRotationQ = vNodeAnim->mRotationKeys[RotationIndex].mValue;
	const aiQuaternion& EndRotationQ = vNodeAnim->mRotationKeys[NextRotationIndex].mValue;
	aiQuaternion::Interpolate(voQuaternion, StartRotationQ, EndRotationQ, Factor);
	voQuaternion = voQuaternion.Normalize();
}

//***********************************************************************************************
//FUNCTION:
unsigned CModel::__findRotation(float vAnimationTime, const aiNodeAnim* vNodeAnim) const
{
	_ASSERTE(vNodeAnim->mNumRotationKeys > 0);

	for (unsigned i = 0; i < vNodeAnim->mNumRotationKeys - 1; ++i)
		if (vAnimationTime < (float)vNodeAnim->mRotationKeys[i + 1].mTime) return i;

	_ASSERTE(false);
	return 0;
}

//***********************************************************************************************
//FUNCTION:
unsigned CModel::__findPosition(float vAnimationTime, const aiNodeAnim* vNodeAnim) const
{
	_ASSERTE(vNodeAnim->mNumPositionKeys > 0);

	for (unsigned i = 0; i < vNodeAnim->mNumPositionKeys - 1; i++)
		if (vAnimationTime < (float)vNodeAnim->mPositionKeys[i + 1].mTime) return i;

	_ASSERTE(false);
	return 0;
}

//***********************************************************************************************
//FUNCTION:
unsigned CModel::__findScaling(float vAnimationTime, const aiNodeAnim* vNodeAnim) const
{
	_ASSERTE(vNodeAnim->mNumScalingKeys > 0);

	for (unsigned i = 0; i < vNodeAnim->mNumScalingKeys - 1; i++)
		if (vAnimationTime < (float)vNodeAnim->mScalingKeys[i + 1].mTime) return i;

	_ASSERTE(false);
	return 0;
}