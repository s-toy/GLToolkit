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

using namespace glt;

//***********************************************************************************************
//FUNCTION:
CModel::CModel(const std::string& vFilePath)
{
	if (!__loadModel(vFilePath))
	{
		_OUTPUT_WARNING("Failed to load model.");
		_ASSERTE(false);
	}
}

//***********************************************************************************************
//FUNCTION:
CModel::~CModel()
{
	for (auto pTexture : m_LoadedTextures) _SAFE_DELETE(pTexture);
}

//**********************************************************************************************
//FUNCTION:
bool CModel::__loadModel(const std::string& vPath)
{
	Assimp::Importer LocImporter;
	const aiScene* pScene = LocImporter.ReadFile(vPath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals);

	if (!pScene || pScene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !pScene->mRootNode) { return false; }

	m_Directory = vPath.substr(0, vPath.find_last_of('/'));
	__processNode(pScene->mRootNode, pScene);

	return true;
}

//**********************************************************************************************
//FUNCTION:
void CModel::__processNode(const aiNode* vNode, const aiScene* vScene)
{
	for (GLuint i = 0; i < vNode->mNumMeshes; ++i)
	{
		aiMesh* mesh = vScene->mMeshes[vNode->mMeshes[i]];
		m_Meshes.push_back(std::move(__processMesh(mesh, vScene)));
	}

	for (GLuint i = 0; i < vNode->mNumChildren; ++i)
	{
		__processNode(vNode->mChildren[i], vScene);
	}
}

//**********************************************************************************************
//FUNCTION:
CMesh CModel::__processMesh(const aiMesh* vMesh, const aiScene* vScene)
{
	std::vector<SVertex> Vertices;
	std::vector<GLuint> Indices;
	std::vector<CTexture2D*> Textures;

	for (GLuint i = 0; i < vMesh->mNumVertices; ++i)
	{
		SVertex TempVertex;
		glm::vec3 TempVector;

		TempVector.x = vMesh->mVertices[i].x;
		TempVector.y = vMesh->mVertices[i].y;
		TempVector.z = vMesh->mVertices[i].z;
		TempVertex.Position = TempVector;

		TempVector.x = vMesh->mNormals[i].x;
		TempVector.y = vMesh->mNormals[i].y;
		TempVector.z = vMesh->mNormals[i].z;
		TempVertex.Normal = TempVector;

		if (vMesh->mTextureCoords[0])
		{
			glm::vec2 Vec;
			Vec.x = vMesh->mTextureCoords[0][i].x;
			Vec.y = vMesh->mTextureCoords[0][i].y;
			TempVertex.TexCoords = Vec;
		}
		else
			TempVertex.TexCoords = glm::vec2(0.0f, 0.0f);
		Vertices.push_back(TempVertex);
	}

	for (GLuint i = 0; i < vMesh->mNumFaces; ++i)
	{
		aiFace Face = vMesh->mFaces[i];

		for (GLuint k = 0; k < Face.mNumIndices; ++k)
			Indices.push_back(Face.mIndices[k]);
	}

	if (vMesh->mMaterialIndex >= 0)
	{
		aiMaterial* pMaterial = vScene->mMaterials[vMesh->mMaterialIndex];

		std::vector<CTexture2D*> DiffuseMaps = __loadMaterialTextures(pMaterial, aiTextureType_DIFFUSE, "uMaterialDiffuse");
		Textures.insert(Textures.end(), DiffuseMaps.begin(), DiffuseMaps.end());

		std::vector<CTexture2D*> SpecularMaps = __loadMaterialTextures(pMaterial, aiTextureType_SPECULAR, "uMaterialSpecular");
		Textures.insert(Textures.end(), SpecularMaps.begin(), SpecularMaps.end());
	}

	return CMesh(Vertices, Indices, Textures);
}

//**********************************************************************************************
//FUNCTION:
std::vector<CTexture2D*> CModel::__loadMaterialTextures(const aiMaterial* vMat, aiTextureType vType, const std::string& vTypeName)
{
	std::vector<CTexture2D*> Textures;
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

			auto pTempTexture = new CTexture2D;
			pTempTexture->load(TexturePath.c_str(), GL_REPEAT, GL_LINEAR, GL_RGBA);
			pTempTexture->setTextureName(vTypeName);
			Textures.push_back(pTempTexture);
			this->m_LoadedTextures.push_back(pTempTexture);
		}
	}
	return Textures;
}

//***********************************************************************************************
//FUNCTION:
void CModel::draw(const CShaderProgram& vShaderProgram) const
{
	for (const auto& Mesh : m_Meshes)
	{
		Mesh.draw(vShaderProgram);
	}
}