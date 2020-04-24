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
	class CShaderProgram;
	class CTexture2D;

	class GLT_DECLSPEC CModel : public CEntity
	{
	public:
		CModel(const std::string& vFilePath);
		~CModel();

	protected:
		void _draw(const CShaderProgram& vShaderProgram) const;

	private:
		void __processNode(const aiNode* vNode, const aiScene* vScene);

		std::shared_ptr<CMesh> __processMesh(const aiMesh* vMesh, const aiScene* vScene);

		std::vector<std::shared_ptr<CTexture2D>> __loadMaterialTextures(const aiMaterial* vMat, aiTextureType vType, const std::string& vTypeName);

		bool __loadModel(const std::string& vPath);

		std::string m_Directory;

		std::vector<std::shared_ptr<CMesh>>			m_Meshes;
		std::vector<std::shared_ptr<CTexture2D>>	m_LoadedTextures;

		static std::unordered_map<std::string, CModel*> m_ExsitedModelMap;

		friend class CRenderer;
	};
}