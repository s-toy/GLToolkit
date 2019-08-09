#include "Mesh.h"
#include <string>
#include <sstream>
#include "ShaderProgram.h"
#include "Renderer.h"

using namespace glt;

//**********************************************************************************************
//FUNCTION:
CMesh::CMesh(std::vector<SVertex> vVertices, std::vector<GLuint> vIndices, std::vector<STexture> vTextures)
{
	m_Vertices = vVertices;
	m_Indices = vIndices;
	m_Textures = vTextures;

	__setupMesh();
}

//**********************************************************************************************
//FUNCTION:
void CMesh::__setupMesh()
{
	m_pVertexBuffer = std::make_unique<CVertexBuffer>(&m_Vertices[0], m_Vertices.size() * sizeof(SVertex));
	m_pIndexBuffer = std::make_unique<CIndexBuffer>(&m_Indices[0], m_Indices.size());
	m_pVertexArray = std::make_unique<CVertexArray>();

	CVertexArrayLayout Layout;
	Layout.push<float>(3);
	Layout.push<float>(3);
	Layout.push<float>(2);
	m_pVertexArray->addBuffer(*m_pVertexBuffer, Layout);
}

//***********************************************************************************************
//FUNCTION:
void CMesh::draw(const CShaderProgram& vShaderProgram) const
{
	m_pVertexArray->bind();
	m_pIndexBuffer->bind();

	for (GLuint i = 0; i < m_Textures.size(); ++i)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, m_Textures[i].Id);
		vShaderProgram.updateUniform1i(m_Textures[i].Type, i);
	}

	glDrawElements(GL_TRIANGLES, m_pIndexBuffer->getCount(), GL_UNSIGNED_INT, nullptr);

#ifdef _DEBUG
	m_pVertexArray->unbind();
	m_pVertexBuffer->unbind();

	for (GLuint i = 0; i < m_Textures.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
#endif
}