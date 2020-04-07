#pragma once
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <assimp/Importer.hpp>
#include "IndexBuffer.h"
#include "VertexArray.h"

namespace glt
{
	typedef struct
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec2 TexCoords;
	} SVertex;

	class CShaderProgram;
	class CTexture2D;

	class CMesh
	{
	public:
		CMesh(std::vector<SVertex> vVertices, std::vector<unsigned int> vIndices, std::vector<CTexture2D*> vTextures);

		void draw(const CShaderProgram& vShaderProgram) const;

	private:
		void __setupMesh();

	private:
		std::vector<SVertex> m_Vertices;
		std::vector<GLuint> m_Indices;
		std::vector<CTexture2D*> m_Textures;

		std::unique_ptr<CVertexBuffer> m_pVertexBuffer;
		std::unique_ptr<CIndexBuffer> m_pIndexBuffer;
		std::unique_ptr<CVertexArray> m_pVertexArray;
	};
}