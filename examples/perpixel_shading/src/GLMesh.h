#pragma once
#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <assimp/Importer.hpp>
#include "GLShader.h"

typedef struct
{
	GLuint Id;
	std::string Type;
	aiString Path;
} STexture;

typedef struct
{
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
} SVertex;

class CGLMesh
{
public:
	CGLMesh(std::vector<SVertex> vVertices, std::vector<GLuint> vIndices, std::vector<STexture> vTextures);

	void draw(GLuint vShaderProgram);

private:
	void __setupMesh();

private:
	std::vector<SVertex> m_Vertices;
	std::vector<GLuint> m_Indices;
	std::vector<STexture> m_Textures;
	GLuint m_VAO, m_VBO, m_EBO;
};