#include "ApplicationBase.h"
#include <GLFW/glfw3.h>
#include "Window.h"
#include "InputManager.h"

using namespace glt;

//*********************************************************************
//FUNCTION:
void glt::CApplicationBase::run()
{
	try
	{
		if (!__init()) _THROW_RUNTIME_ERROR("Failed to run application due to failure of initialization!");

		_OUTPUT_EVENT("Succeed to init application.");

		while (!glfwWindowShouldClose(_pWindow->getGLFWWindow()))
		{
			CRenderer::getInstance()->update();
			_updateV();

			glfwSwapBuffers(_pWindow->getGLFWWindow());
			glfwPollEvents();
		}

		__destroy();

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
bool glt::CApplicationBase::__init()
{
	_EARLY_RETURN(!glfwInit(), "Failed to initialize glfw.", false);

#ifdef _DEBUG
	glfwSetErrorCallback(__glfwErrorCallback);
#endif

	_pWindow = new CWindow;
	_EARLY_RETURN(!_pWindow->createWindow(m_WindowInfo), "Failed to create window.", false);

	_EARLY_RETURN(!CRenderer::getInstance()->init(), "Failed to initialize renderer.", false);
	CRenderer::getInstance()->fetchCamera()->setAspect((double)m_WindowInfo.Width / m_WindowInfo.Height);

	CInputManager::getInstance()->init(_pWindow->getGLFWWindow());

	if (!_initV()) return false;

	return true;
}

//*********************************************************************
//FUNCTION:
void glt::CApplicationBase::__destroy()
{
	_destroyV();

	_SAFE_DELETE(_pWindow);

	CRenderer::getInstance()->destroy();

	glfwTerminate();
}