#include "Mesh.h"
#include <string>
#include <sstream>

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
void CMesh::draw(GLuint vShaderProgram)
{
	GLuint DiffuseNr = 1;
	GLuint SpecularNr = 1;
	for (GLuint i = 0; i < m_Textures.size(); ++i)
	{
		glActiveTexture(GL_TEXTURE0 + i);

		std::stringstream SStream;
		std::string Number;
		std::string Name = m_Textures[i].Type;
		SStream << Name;
		if (Name == "uMaterialDiffuse")
		{
			SStream << DiffuseNr++;
		}
		else if (Name == "uMaterialSpecular")
		{
			SStream << SpecularNr++;
		}
		glUniform1i(glGetUniformLocation(vShaderProgram, SStream.str().c_str()), i);

		glBindTexture(GL_TEXTURE_2D, m_Textures[i].Id);
	}

	m_pVertexArray->bind();
	m_pIndexBuffer->bind();
	glDrawElements(GL_TRIANGLES, m_Indices.size(), GL_UNSIGNED_INT, 0);

	for (GLuint i = 0; i < m_Textures.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
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