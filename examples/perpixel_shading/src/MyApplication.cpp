#include "MyApplication.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "glt/camera.h"
#include "GLModel.h"
#include "GLShader.h"

using namespace glt;

namespace
{
	const char* VERT_SHADER_PATH = "shaders/perpixel_shading_vs.glsl";
	const char* FRAG_SHADER_PATH = "shaders/perpixel_shading_fs.glsl";
	const char* MODEL_PATH = "../../resource/models/nanosuit/nanosuit.obj";
	const GLuint SCREEN_WIDTH = 1600, SCREEN_HEIGHT = 900;

	CGLModel g_Model;
	GLuint g_ShaderProgram;

	glm::vec3 g_PointLightPositions[] = {
		glm::vec3(2.3f, -1.6f, -3.0f),
		glm::vec3(-1.7f, 0.9f, 1.0f)
	};
}

//***********************************************************************************************
//FUNCTION:
bool CMyApplication::_initV()
{
	if (!CApplicationBase::_initV()) return false;

	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

	SShaderInfo Shaders[] =
	{
		{ GL_VERTEX_SHADER, VERT_SHADER_PATH },
		{ GL_FRAGMENT_SHADER, FRAG_SHADER_PATH },
		{ GL_NONE, NULL }
	};
	CGLShader ShaderObj;
	ShaderObj.loadShaders(Shaders);
	ShaderObj.useProgram();
	g_ShaderProgram = ShaderObj.getProgram();

	g_Model.loadModel(MODEL_PATH);

	return true;
}

//***********************************************************************************************
//FUNCTION:
void CMyApplication::_updateV()
{
	CApplicationBase::_updateV();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	auto pCamera = getCamera();
	glUniform3f(glGetUniformLocation(g_ShaderProgram, "uViewPos"), pCamera->getPosition().x, pCamera->getPosition().y, pCamera->getPosition().z);

	glm::mat4 Projection = pCamera->getProjectionMatrix();
	glm::mat4 View = pCamera->getViewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(g_ShaderProgram, "uProjectionMatrix"), 1, GL_FALSE, glm::value_ptr(Projection));
	glUniformMatrix4fv(glGetUniformLocation(g_ShaderProgram, "uViewMatrix"), 1, GL_FALSE, glm::value_ptr(View));

	glm::mat4 Model;
	Model = glm::translate(Model, glm::vec3(0.0f, -1.5f, 0.0f));
	Model = glm::scale(Model, glm::vec3(0.2f, 0.2f, 0.2f));
	glUniformMatrix4fv(glGetUniformLocation(g_ShaderProgram, "uModelMatrix"), 1, GL_FALSE, glm::value_ptr(Model));

	g_Model.draw(g_ShaderProgram);

	glFlush();
}

//***********************************************************************************************
//FUNCTION:
void CMyApplication::_destroyV()
{
	CApplicationBase::_destroyV();
}