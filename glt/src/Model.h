#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <assimp/scene.h>
#include "Entity.h"
#include "Mesh.h"
#include "Export.h"

namespace glt
{
	struct SBoneInfo
	{
		glm::mat4 BoneOffset;
		glm::mat4 FinalTransformation;
	};

	class CShaderProgram;
	class CTexture2D;

	class GLT_DECLSPEC CModel : public CEntity
	{
	public:
		CModel(const std::string& vFilePath);
		~CModel();

		SAABB getAABB() const;

	protected:
		void _draw(const CShaderProgram& vShaderProgram) const;
		bool _hasBones() const { return m_HasBones; }
		void _boneTransform(float vTimeInSeconds, std::vector<glm::mat4>& voTransforms) const;

	private:
		void __processNode(const aiNode* vNode);
		std::shared_ptr<CMesh> __processMesh(const aiMesh* vMesh);
		std::vector<std::shared_ptr<CTexture2D>> __loadMaterialTextures(const aiMaterial* vMat, aiTextureType vType, const std::string& vTypeName);
		bool __loadModel(const std::string& vPath);

		void __readNodeHeirarchy(float vAnimationTime, const aiNode* vNode, const glm::mat4& vParentTransform) const;
		const aiNodeAnim* __findNodeAnim(const aiAnimation* vAnimation, const std::string vNodeName) const;
		void __calcInterpolatedPosition(aiVector3D& voVector, float vAnimationTime, const aiNodeAnim* vNodeAnim) const;
		void __calcInterpolatedRotation(aiQuaternion& voQuaternion, float vAnimationTime, const aiNodeAnim* vNodeAnim) const;
		void __calcInterpolatedScaling(aiVector3D& voVector, float vAnimationTime, const aiNodeAnim* vNodeAnim) const;
		unsigned __findPosition(float vAnimationTime, const aiNodeAnim* vNodeAnim) const;
		unsigned __findRotation(float vAnimationTime, const aiNodeAnim* vNodeAnim) const;
		unsigned __findScaling(float vAnimationTime, const aiNodeAnim* vNodeAnim) const;

		std::vector<std::shared_ptr<CMesh>>			m_Meshes;
		std::vector<std::shared_ptr<CTexture2D>>	m_LoadedTextures;

		std::shared_ptr<Assimp::Importer> m_pImporter;
		const aiScene* m_pScene = nullptr;
		std::string m_Directory;

		std::unordered_map<std::string, unsigned>		m_BoneName2IndexMap;
		mutable std::shared_ptr<std::vector<SBoneInfo>>	m_pBoneInfo;
		unsigned	m_NumBones = 0;
		bool		m_HasBones = false;
		glm::mat4	m_GlobalInverseTransform;

		static std::unordered_map<std::string, CModel*> m_ExsitedModelMap;

		friend class CRenderer;
	};
}