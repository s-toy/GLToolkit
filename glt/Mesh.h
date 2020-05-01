#pragma once
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <assimp/Importer.hpp>
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "Common.h"

namespace glt
{
	typedef struct
	{
		glm::vec3  Position;
		glm::vec3  Normal;
		glm::vec2  TexCoords;
		glm::ivec4 BoneIDs;
		glm::vec4  BoneWeights;
	} SVertex;

	class CShaderProgram;
	class CTexture2D;

	class CMesh
	{
	public:
		CMesh(const std::vector<SVertex>& vVertices, const std::vector<unsigned int>& vIndices, const std::vector<std::shared_ptr<CTexture2D>>& vTextures);

	protected:
		void _draw(const CShaderProgram& vShaderProgram) const;

	private:
		void __setupMesh();

	private:
		std::vector<SVertex> m_Vertices;
		std::vector<GLuint> m_Indices;
		std::vector<std::shared_ptr<CTexture2D>> m_Textures;

		std::shared_ptr<CVertexBuffer>	m_pVertexBuffer;
		std::shared_ptr<CIndexBuffer>	m_pIndexBuffer;
		std::shared_ptr<CVertexArray>	m_pVertexArray;

		friend class CModel;
	};
}