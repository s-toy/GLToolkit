#include "MyApplication.h"
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
	m_pModel->setPosition(glm::vec3(0.0f, -1.5f, 0.0f));
	m_pModel->setScale(glm::vec3(0.2f, 0.2f, 0.2f));

	CRenderer::getInstance()->fetchCamera()->setPosition(glm::dvec3(0, 0, 5));

	return true;
}

//***********************************************************************************************
//FUNCTION:
void CMyApplication::_updateV()
{
	CRenderer::getInstance()->clear();
	CRenderer::getInstance()->draw(*m_pModel, *m_pShaderProgram);
}

//***********************************************************************************************
//FUNCTION:
void CMyApplication::_destroyV()
{
	_SAFE_DELETE(m_pShaderProgram);
	_SAFE_DELETE(m_pModel);
}