#include "MyApplication.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Camera.h"
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

	m_pModel = new CModel;
	m_pModel->loadModel("../../resource/models/lpshead/head.obj");

	_pCamera->setPosition(glm::dvec3(0, 0, 30));
	_pCamera->setNearPlane(0.01);

	return true;
}

//***********************************************************************************************
//FUNCTION:
void CMyApplication::_updateV()
{
	m_pShaderProgram->enable();
	m_pShaderProgram->setUniform3f("uViewPos", _pCamera->getPosition());
	m_pShaderProgram->setUniformMat4("uProjectionMatrix", _pCamera->getProjectionMatrix());
	m_pShaderProgram->setUniformMat4("uViewMatrix", _pCamera->getViewMatrix());

	auto ModelMatrix = glm::translate(glm::mat4(1.0), glm::vec3(0.0f, 0.0f, 0.0f));
	m_pShaderProgram->setUniformMat4("uModelMatrix", ModelMatrix);

	m_pModel->draw(m_pShaderProgram->getProgramID());

	m_pShaderProgram->disable();
}

//***********************************************************************************************
//FUNCTION:
void CMyApplication::_destroyV()
{
	_SAFE_DELETE(m_pShaderProgram);
	_SAFE_DELETE(m_pModel);
}