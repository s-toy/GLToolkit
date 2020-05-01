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

	protected:
		void _draw(const CShaderProgram& vShaderProgram) const;
		bool _hasBones() const { return m_HasBones; }

	private:
		void __processNode(const aiNode* vNode, const aiScene* vScene);
		std::shared_ptr<CMesh> __processMesh(const aiMesh* vMesh, const aiScene* vScene);
		std::vector<std::shared_ptr<CTexture2D>> __loadMaterialTextures(const aiMaterial* vMat, aiTextureType vType, const std::string& vTypeName);
		bool __loadModel(const std::string& vPath);

		void __boneTransform(float vTimeInSeconds, std::vector<glm::mat4>& voTransforms);
		void __readNodeHeirarchy(float vAnimationTime, const aiNode* vNode, const glm::mat4& vParentTransform);
		const aiNodeAnim* __findNodeAnim(const aiAnimation* vAnimation, const std::string vNodeName);
		void __calcInterpolatedPosition(aiVector3D& voVector, float vAnimationTime, const aiNodeAnim* vNodeAnim);
		void __calcInterpolatedRotation(aiQuaternion& voQuaternion, float vAnimationTime, const aiNodeAnim* vNodeAnim);
		void __calcInterpolatedScaling(aiVector3D& voVector, float vAnimationTime, const aiNodeAnim* vNodeAnim);
		unsigned __findPosition(float vAnimationTime, const aiNodeAnim* vNodeAnim);
		unsigned __findRotation(float vAnimationTime, const aiNodeAnim* vNodeAnim);
		unsigned __findScaling(float vAnimationTime, const aiNodeAnim* vNodeAnim);

		std::vector<std::shared_ptr<CMesh>>			m_Meshes;
		std::vector<std::shared_ptr<CTexture2D>>	m_LoadedTextures;
		const aiScene* m_pScene = nullptr;
		std::string m_Directory;

		std::unordered_map<std::string, unsigned>	m_BoneName2IndexMap;
		std::vector<SBoneInfo>						m_BoneInfo;
		unsigned	m_NumBones = 0;
		bool		m_HasBones = false;
		glm::mat4 m_GlobalInverseTransform;

		static std::unordered_map<std::string, CModel*> m_ExsitedModelMap;

		friend class CRenderer;
	};
}