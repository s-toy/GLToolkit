#include "Renderer.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "VertexArray.h"
#include "IndexBuffer.h"
#include "VertexBuffer.h"
#include "ShaderProgram.h"
#include "Model.h"
#include "DebugUtil.h"
#include "Skybox.h"

using namespace glt;

//***********************************************************************************************
//FUNCTION:
bool CRenderer::init()
{
	_EARLY_RETURN(!gladLoadGL(), "Failed to initialize GLAD.", false);

#ifdef _DEBUG
	GLint Flags; glGetIntegerv(GL_CONTEXT_FLAGS, &Flags);
	if (Flags & GL_CONTEXT_FLAG_DEBUG_BIT)
	{
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(gltDebugCallback, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	}
#endif // DEBUG

	glEnable(GL_DEPTH_TEST);

	m_pCamera = new CCamera;

	return true;
}

//***********************************************************************************************
//FUNCTION:
void CRenderer::destroy()
{
	_SAFE_DELETE(m_pCamera);
}

//***********************************************************************************************
//FUNCTION:
void CRenderer::clear() const
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

//*********************************************************************
//FUNCTION:
void CRenderer::draw(const CVertexArray& vVertexArray, const CIndexBuffer& vIndexBuffer, const CShaderProgram& vShaderProgram) const
{
	vVertexArray.bind();
	vIndexBuffer.bind();
	vShaderProgram.bind();

	__updateShaderUniform(vShaderProgram);
	glDrawElements(GL_TRIANGLES, vIndexBuffer.getCount(), GL_UNSIGNED_INT, nullptr);

#ifdef _DEBUG
	vVertexArray.unbind();
	vIndexBuffer.unbind();
	vShaderProgram.unbind();
#endif // DEBUG
}

//***********************************************************************************************
//FUNCTION:
void CRenderer::__drawSingleModel(const CModel& vModel, const CShaderProgram& vShaderProgram)
{
	auto ModelMatrix = glm::translate(glm::mat4(1.0), vModel.getPosition());
	ModelMatrix = glm::scale(ModelMatrix, vModel.getScale());
	vShaderProgram.updateUniformMat4("uModelMatrix", ModelMatrix);

	vModel._draw(vShaderProgram);
}

//***********************************************************************************************
//FUNCTION:
void CRenderer::draw(const CModel& vModel, const CShaderProgram& vShaderProgram)
{
	vShaderProgram.bind();

	__updateShaderUniform(vShaderProgram);
	__drawSingleModel(vModel, vShaderProgram);

#ifdef _DEBUG
	vShaderProgram.unbind();
#endif
}

//***********************************************************************************************
//FUNCTION:
void CRenderer::draw(const std::vector<std::shared_ptr<CModel>>& vModels, const CShaderProgram& vShaderProgram)
{
	vShaderProgram.bind();

	__updateShaderUniform(vShaderProgram);
	for (const auto& Model : vModels) __drawSingleModel(*Model, vShaderProgram);

#ifdef _DEBUG
	vShaderProgram.unbind();
#endif
}

//***********************************************************************************************
//FUNCTION:
void CRenderer::drawScreenQuad(const CShaderProgram& vShaderProgram)
{
	if (!m_FullScreenQuadVAO) __initFullScreenQuad();

	vShaderProgram.bind();
	vShaderProgram.updateUniform3f("uViewPos", m_pCamera->getPosition());

	m_FullScreenQuadVAO->bind();
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

#ifdef _DEBUG
	m_FullScreenQuadVAO->unbind();
	vShaderProgram.unbind();
#endif
}

//***********************************************************************************************
//FUNCTION:
void CRenderer::drawSkybox(const CSkybox& vSkybox, unsigned int vBindPoint)
{
	auto pShaderProgram = vSkybox._getShaderProgram();
	pShaderProgram->bind();
	pShaderProgram->updateUniformMat4("uProjectionMatrix", m_pCamera->getProjectionMatrix());
	pShaderProgram->updateUniformMat4("uViewMatrix", m_pCamera->getViewMatrix());
	vSkybox._draw(vBindPoint);
}

//***********************************************************************************************
//FUNCTION:
void CRenderer::update()
{
	m_pCamera->update();
}

//***********************************************************************************************
//FUNCTION:
void CRenderer::__updateShaderUniform(const CShaderProgram& vShaderProgram) const
{
	vShaderProgram.updateUniform3f("uViewPos", m_pCamera->getPosition());
	vShaderProgram.updateUniformMat4("uProjectionMatrix", m_pCamera->getProjectionMatrix());
	vShaderProgram.updateUniformMat4("uViewMatrix", m_pCamera->getViewMatrix());
}

//***********************************************************************************************
//FUNCTION:
void CRenderer::__initFullScreenQuad()
{
	GLfloat VertexData[] = { -1.0f, -1.0f,	 1.0f, -1.0f,	 -1.0f, 1.0f,	 1.0f, 1.0f };

	m_FullScreenQuadVAO = std::make_shared<CVertexArray>();
	m_FullScreenQuadVAO->bind();

	m_FullScreenQuadVBO = std::make_shared<CVertexBuffer>(VertexData, sizeof(VertexData));
	CVertexArrayLayout VertexArrayLayout;
	VertexArrayLayout.push<float>(2);
	m_FullScreenQuadVAO->addBuffer(*m_FullScreenQuadVBO, VertexArrayLayout);

#ifdef _DEBUG
	m_FullScreenQuadVAO->unbind();
#endif // _DEBUG
}