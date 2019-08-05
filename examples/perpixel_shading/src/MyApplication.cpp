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
	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, _WindowInfo.Width, _WindowInfo.Height);

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

	_pCamera->setPosition(glm::dvec3(0, 0, 3));

	return true;
}

//***********************************************************************************************
//FUNCTION:
void CMyApplication::_updateV()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUniform3f(glGetUniformLocation(g_ShaderProgram, "uViewPos"), _pCamera->getPosition().x, _pCamera->getPosition().y, _pCamera->getPosition().z);

	glm::mat4 Projection = _pCamera->getProjectionMatrix();
	glm::mat4 View = _pCamera->getViewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(g_ShaderProgram, "uProjectionMatrix"), 1, GL_FALSE, glm::value_ptr(Projection));
	glUniformMatrix4fv(glGetUniformLocation(g_ShaderProgram, "uViewMatrix"), 1, GL_FALSE, glm::value_ptr(View));

	auto Model = glm::translate(glm::mat4(1.0), glm::vec3(0.0f, -1.5f, 0.0f));
	Model = glm::scale(Model, glm::vec3(0.2f, 0.2f, 0.2f));
	glUniformMatrix4fv(glGetUniformLocation(g_ShaderProgram, "uModelMatrix"), 1, GL_FALSE, glm::value_ptr(Model));

	g_Model.draw(g_ShaderProgram);

	glFlush();
}

//***********************************************************************************************
//FUNCTION:
void CMyApplication::_destroyV()
{

}