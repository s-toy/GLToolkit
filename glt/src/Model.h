#pragma once
#include <string>
#include <vector>
#include <assimp/scene.h>
#include "Entity.h"
#include "Mesh.h"

namespace glt
{
	class CShaderProgram;

	class CModel : public CEntity
	{
	public:
		CModel(const std::string& vFilePath);

		void draw(const CShaderProgram& vShaderProgram) const;

	private:
		void __processNode(const aiNode* vNode, const aiScene* vScene);

		CMesh __processMesh(const aiMesh* vMesh, const aiScene* vScene);

		std::vector<STexture> __loadMaterialTextures(const aiMaterial* vMat, aiTextureType vType, const std::string& vTypeName);

		bool __loadModel(const std::string& vPath);

		std::vector<CMesh> m_Meshes;
		std::string m_Directory;
		std::vector<STexture> m_LoadedTextures;
	};
}