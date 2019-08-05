#include "ApplicationBase.h"
#include "Window.h"
#include "Camera.h"
#include "InputManager.h"
#include "DebugUtil.h"

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

		while (!glfwWindowShouldClose(_pWindow->getGLFWWindow()))
		{
			_updateV();
			glfwSwapBuffers(_pWindow->getGLFWWindow());
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

//*********************************************************************
//FUNCTION:
bool glt::CApplicationBase::_initV()
{
	if (!glfwInit()) return false;

#ifdef _DEBUG
	glfwSetErrorCallback(debug_util::glfwErrorCallback);
#endif

	_pWindow = new CWindow;
	if (!_pWindow->createWindow(_WindowInfo)) return false;

	CInputManager::getInstance()->init(_pWindow->getGLFWWindow());

	glewExperimental = GL_TRUE;
	if (GLEW_OK != glewInit()) return false;

#ifdef _DEBUG
	GLint flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
	{
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(debug_util::glDebugCallback, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	}
#endif // DEBUG

	_pCamera = new CCamera();
	_pCamera->setAspect(double(_WindowInfo.Width) / _WindowInfo.Height);

	return true;
}

//***********************************************************************************************
//FUNCTION:
void glt::CApplicationBase::_updateV()
{
	_pCamera->update();
}

//*********************************************************************
//FUNCTION:
void glt::CApplicationBase::_destroyV()
{
	delete _pCamera;
	delete _pWindow;

	glfwTerminate();
}