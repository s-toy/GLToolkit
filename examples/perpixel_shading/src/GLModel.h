#pragma once

#include <string>
#include <vector>
#include <GL/glew.h>
#include <assimp/scene.h>
#include "GLMesh.h"

class CGLModel 
{
public:
	void draw(GLuint vShaderProgram);

	bool loadModel(const std::string& vPath);

private:
	void __processNode(const aiNode* vNode, const aiScene* vScene);

	CGLMesh __processMesh(const aiMesh* vMesh, const aiScene* vScene);

	std::vector<STexture> __loadMaterialTextures(const aiMaterial* vMat, aiTextureType vType, const std::string& vTypeName);

private:
	std::vector<CGLMesh> m_Meshes;
	std::string m_Directory;
	std::vector<STexture> m_LoadedTextures;
};
