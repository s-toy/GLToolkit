#pragma once
#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <assimp/Importer.hpp>

namespace glt
{
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

	class CMesh
	{
	public:
		CMesh(std::vector<SVertex> vVertices, std::vector<GLuint> vIndices, std::vector<STexture> vTextures);

		void draw(GLuint vShaderProgram);

	private:
		void __setupMesh();

	private:
		std::vector<SVertex> m_Vertices;
		std::vector<GLuint> m_Indices;
		std::vector<STexture> m_Textures;
		GLuint m_VAO, m_VBO, m_EBO;
	};
}