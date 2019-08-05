#include "ApplicationBase.h"
#include "Window.h"
#include "Camera.h"
#include "InputManager.h"

using namespace glt;

CApplicationBase::CApplicationBase()
{
}

CApplicationBase::~CApplicationBase()
{
}

//*********************************************************************
//FUNCTION:
void glt::CApplicationBase::run()
{
	try
	{
		if (!_initV()) _THROW_RUNTIME_ERROR("Failed to run application due to failure of initialization!");

		_OUTPUT_EVENT("Succeed to init application.");

		while (!glfwWindowShouldClose(m_pWindow->getGLFWWindow()))
		{
			_updateV();
			glfwSwapBuffers(m_pWindow->getGLFWWindow());
			glfwPollEvents();
		}

		_destroyV();

		_OUTPUT_EVENT("Succeed to end application.");
	}
	catch (const std::runtime_error& e)
	{
		_OUTPUT_WARNING(e.what());
		exit(EXIT_FAILURE);
	}
	catch (...)
	{
		_OUTPUT_WARNING("The program is terminated due to unexpected error!");
		exit(EXIT_FAILURE);
	}
}

//***********************************************************************************
//FUNCTION:
#ifdef _DEBUG
static void __glfwErrorCallback(int vError, const char* vDescription)
{
	_OUTPUT_WARNING(std::string("GLFW error: ") + vDescription);
}
#endif

//*********************************************************************
//FUNCTION:
bool glt::CApplicationBase::_initV()
{
	if (!glfwInit()) return false;

#ifdef _DEBUG
	glfwSetErrorCallback(__glfwErrorCallback);
#endif

	m_pWindow = new CWindow;
	if (!m_pWindow->createWindow(m_WindowInfo)) return false;

	CInputManager::getInstance()->init(m_pWindow->getGLFWWindow());

	glewExperimental = GL_TRUE;
	if (GLEW_OK != glewInit()) return false;

	m_pCamera = new CCamera;
	m_pCamera->setAspect(double(m_WindowInfo.Width) / m_WindowInfo.Height);

	return true;
}

//***********************************************************************************************
//FUNCTION:
void glt::CApplicationBase::_updateV()
{
	m_pCamera->update();
}

//*********************************************************************
//FUNCTION:
void glt::CApplicationBase::_destroyV()
{
	glfwTerminate();

	delete m_pCamera;
	delete m_pWindow;
}