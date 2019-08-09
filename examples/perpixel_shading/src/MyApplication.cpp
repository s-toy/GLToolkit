#include "MyApplication.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "ShaderProgram.h"
#include "Model.h"

using namespace glt;

//***********************************************************************************************
//FUNCTION:
bool CMyApplication::_initV()
{
	m_pShaderProgram = new CShaderProgram;
	m_pShaderProgram->addShader("shaders/perpixel_shading_vs.glsl", EShaderType::VERTEX_SHADER);
	m_pShaderProgram->addShader("shaders/perpixel_shading_fs.glsl", EShaderType::FRAGMENT_SHADER);

	m_pModel = new CModel("../../resource/models/nanosuit/nanosuit.obj");

	_pCamera->setPosition(glm::dvec3(0, 0, 5));

	return true;
}

//***********************************************************************************************
//FUNCTION:
void CMyApplication::_updateV()
{
	CRenderer::getInstance()->clear();

	m_pShaderProgram->bind();
	m_pShaderProgram->updateUniform3f("uViewPos", _pCamera->getPosition());
	m_pShaderProgram->updateUniformMat4("uProjectionMatrix", _pCamera->getProjectionMatrix());
	m_pShaderProgram->updateUniformMat4("uViewMatrix", _pCamera->getViewMatrix());

	auto ModelMatrix = glm::translate(glm::mat4(1.0), glm::vec3(0.0f, -1.5f, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(0.2f, 0.2f, 0.2f));
	m_pShaderProgram->updateUniformMat4("uModelMatrix", ModelMatrix);

	CRenderer::getInstance()->draw(*m_pModel, *m_pShaderProgram);

	m_pShaderProgram->unbind();
}

//***********************************************************************************************
//FUNCTION:
void CMyApplication::_destroyV()
{
	_SAFE_DELETE(m_pShaderProgram);
	_SAFE_DELETE(m_pModel);
}