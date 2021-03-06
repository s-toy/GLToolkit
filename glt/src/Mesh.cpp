#include "Mesh.h"
#include <string>
#include <sstream>
#include "ShaderProgram.h"
#include "Renderer.h"
#include "Texture.h"

using namespace glt;

//**********************************************************************************************
//FUNCTION:
CMesh::CMesh(const std::vector<SVertex>& vVertices, const std::vector<unsigned int>& vIndices, const std::vector<std::shared_ptr<CTexture2D>>& vTextures,
	const std::vector<SUniformInfo>& vUniforms, const SAABB& vAABB)
{
	m_Vertices = vVertices;
	m_Indices = vIndices;
	m_Textures = vTextures;
	m_Uniforms = vUniforms;
	m_AABB = vAABB;

	__setupMesh();
}

//**********************************************************************************************
//FUNCTION:
void CMesh::__setupMesh()
{
	m_pVertexBuffer = std::make_shared<CVertexBuffer>(&m_Vertices[0], m_Vertices.size() * sizeof(SVertex));
	m_pIndexBuffer = std::make_shared<CIndexBuffer>(&m_Indices[0], m_Indices.size());
	m_pVertexArray = std::make_shared<CVertexArray>();

	CVertexArrayLayout Layout;
	Layout.push<float>(3);
	Layout.push<float>(3);
	Layout.push<float>(2);
	Layout.push<int>(4);
	Layout.push<float>(4);
	m_pVertexArray->addBuffer(*m_pVertexBuffer, Layout);
}

//***********************************************************************************************
//FUNCTION:
void CMesh::_draw(const CShaderProgram& vShaderProgram) const
{
	m_pVertexArray->bind();
	m_pIndexBuffer->bind();

	for (int i = 0; i < m_Textures.size(); ++i)
	{
		m_Textures[i]->bindV(i);
		vShaderProgram.updateUniform1i(m_Textures[i]->getTextureName(), i);
	}

	for (int i = 0; i < m_Uniforms.size(); ++i)
	{
		switch (m_Uniforms[i].Type)
		{
		case EUniformType::FLOAT:
			vShaderProgram.updateUniform1f(m_Uniforms[i].Name, std::any_cast<float>(m_Uniforms[i].Value));
			break;
		case EUniformType::VEC2F:
			vShaderProgram.updateUniform2f(m_Uniforms[i].Name, std::any_cast<glm::vec2>(m_Uniforms[i].Value));
			break;
		case EUniformType::VEC3F:
			vShaderProgram.updateUniform3f(m_Uniforms[i].Name, std::any_cast<glm::vec3>(m_Uniforms[i].Value));
			break;
		case EUniformType::VEC4F:
			vShaderProgram.updateUniform4f(m_Uniforms[i].Name, std::any_cast<glm::vec4>(m_Uniforms[i].Value));
			break;
		default:
			_OUTPUT_WARNING("The uniform type is not supported.");
			break;
		}
	}

	glDrawElements(GL_TRIANGLES, m_pIndexBuffer->getCount(), GL_UNSIGNED_INT, nullptr);

#ifdef _DEBUG
	m_pVertexArray->unbind();
	m_pVertexBuffer->unbind();

	for (GLuint i = 0; i < m_Textures.size(); i++)
	{
		m_Textures[i]->unbindV();
	}
#endif
}