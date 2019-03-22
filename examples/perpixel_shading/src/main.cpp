#include <string>
#include <fstream>
#include <iostream>
#include <cassert>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "GLModel.h"
#include "GLShader.h"
#include "GLCamera.h"

using namespace std;

namespace
{
	const char* VERT_SHADER_PATH = "shaders/perpixel_shading_vs.glsl";
	const char* FRAG_SHADER_PATH = "shaders/perpixel_shading_fs.glsl";
	const char* MODEL_PATH = "../../resource/models/nanosuit/nanosuit.obj";
	const GLuint SCREEN_WIDTH = 1600, SCREEN_HEIGHT = 900;

	CGLCamera g_Camera(glm::vec3(0.0f, 0.0f, 3.0f));
	CGLModel g_Model;
	GLuint g_ShaderProgram;

	GLfloat g_DeltaTime = 0.0f;
	GLfloat g_LastFrame = 0.0f;
	bool g_Keys[1024];
	bool g_Buttons[3];

	glm::vec3 g_PointLightPositions[] = {
		glm::vec3(2.3f, -1.6f, -3.0f),
		glm::vec3(-1.7f, 0.9f, 1.0f)
	};
}

void init();
void update();
void keyCallback(GLFWwindow*, int, int, int, int);
void mouseCallback(GLFWwindow*, double, double);
void mouseButtonCallback(GLFWwindow*, int, int, int);
void scrollCallback(GLFWwindow*, double, double);
void doMovement();

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "OpenGL_Exercise", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	glfwSetKeyCallback(window, keyCallback);
	glfwSetCursorPosCallback(window, mouseCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	glfwSetScrollCallback(window, scrollCallback);
	glewExperimental = GL_TRUE;

	glewInit();
	init();
	while (!glfwWindowShouldClose(window))
	{
		update();
		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}

//**********************************************************************************************
//FUNCTION:
void init()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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
}

//**********************************************************************************************
//FUNCTION:
void update()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	GLfloat CurrentFrame = glfwGetTime();
	g_DeltaTime = CurrentFrame - g_LastFrame;
	g_LastFrame = CurrentFrame;

	glfwPollEvents();
	doMovement();

	glUniform3f(glGetUniformLocation(g_ShaderProgram, "uViewPos"), g_Camera.Position.x, g_Camera.Position.y, g_Camera.Position.z);

	glm::mat4 Projection = glm::perspective(g_Camera.Zoom, (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);
	glm::mat4 View = g_Camera.getViewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(g_ShaderProgram, "uProjectionMatrix"), 1, GL_FALSE, glm::value_ptr(Projection));
	glUniformMatrix4fv(glGetUniformLocation(g_ShaderProgram, "uViewMatrix"), 1, GL_FALSE, glm::value_ptr(View));

	glm::mat4 Model;
	Model = glm::translate(Model, glm::vec3(0.0f, -1.5f, 0.0f));
	Model = glm::scale(Model, glm::vec3(0.2f, 0.2f, 0.2f));
	glUniformMatrix4fv(glGetUniformLocation(g_ShaderProgram, "uModelMatrix"), 1, GL_FALSE, glm::value_ptr(Model));

	g_Model.draw(g_ShaderProgram);

	glFlush();
}

//**********************************************************************************************
//FUNCTION:
void doMovement()
{
	if (g_Keys[GLFW_KEY_W])
		g_Camera.processKeyboard(FORWARD, g_DeltaTime);
	if (g_Keys[GLFW_KEY_S])
		g_Camera.processKeyboard(BACKWARD, g_DeltaTime);
	if (g_Keys[GLFW_KEY_A])
		g_Camera.processKeyboard(LEFT, g_DeltaTime);
	if (g_Keys[GLFW_KEY_D])
		g_Camera.processKeyboard(RIGHT, g_DeltaTime);
	if (g_Keys[GLFW_KEY_Q])
		g_Camera.processKeyboard(UP, g_DeltaTime);
	if (g_Keys[GLFW_KEY_E])
		g_Camera.processKeyboard(DOWN, g_DeltaTime);
}

//**********************************************************************************************
//FUNCTION:
void keyCallback(GLFWwindow* vWindow, int vKey, int vScancode, int vAction, int vMode)
{
	if (vKey == GLFW_KEY_ESCAPE && vAction == GLFW_PRESS)
		glfwSetWindowShouldClose(vWindow, GL_TRUE);
	if (vKey >= 0 && vKey < 1024) {
		if (vAction == GLFW_PRESS)
			g_Keys[vKey] = true;
		else if (vAction == GLFW_RELEASE)
			g_Keys[vKey] = false;
	}
}

//**********************************************************************************************
//FUNCTION:
void mouseButtonCallback(GLFWwindow* vWndow, int vButton, int vAction, int vMods)
{
	if (vButton >= 0 && vButton < 3) {
		if (vAction == GLFW_PRESS)
			g_Buttons[vButton] = true;
		else if (vAction == GLFW_RELEASE)
			g_Buttons[vButton] = false;
	}
}

//**********************************************************************************************
//FUNCTION:
void mouseCallback(GLFWwindow* window, double xpos, double ypos)
{

	static bool FirstMouse = true;
	static GLfloat LastX = SCREEN_WIDTH / 2.0;
	static GLfloat LastY = SCREEN_HEIGHT / 2.0;
	if (FirstMouse)
	{
		LastX = xpos;
		LastY = ypos;
		FirstMouse = false;
	}

	GLfloat xoffset = xpos - LastX;
	GLfloat yoffset = LastY - ypos;  // Reversed since y-coordinates go from bottom to left

	LastX = xpos;
	LastY = ypos;

	if (g_Buttons[GLFW_MOUSE_BUTTON_LEFT]) g_Camera.processMouseMovement(xoffset, yoffset);
}

//**********************************************************************************************
//FUNCTION:
void scrollCallback(GLFWwindow* vWindow, double vXOffset, double vYOffset)
{
	g_Camera.processMouseScroll(vYOffset);
}