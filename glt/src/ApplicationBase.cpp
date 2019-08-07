#include "ApplicationBase.h"
#include "Window.h"
#include "Camera.h"
#include "InputManager.h"
#include "DebugUtil.h"

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
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			_pCamera->update();
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

//*********************************************************************
//FUNCTION:
bool glt::CApplicationBase::__init()
{
	_EARLY_RETURN(!glfwInit(), "Failed to initialize glfw.", false);

#ifdef _DEBUG
	glfwSetErrorCallback(debug_util::glfwErrorCallback);
#endif

	_pWindow = new CWindow;
	_EARLY_RETURN(!_pWindow->createWindow(m_WindowInfo), "Failed to create window.", false);

	CInputManager::getInstance()->init(_pWindow->getGLFWWindow());

	_EARLY_RETURN(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress), "Failed to initialize GLAD.", false);

#ifdef _DEBUG
	GLint Flags; glGetIntegerv(GL_CONTEXT_FLAGS, &Flags);
	if (Flags & GL_CONTEXT_FLAG_DEBUG_BIT)
	{
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(debug_util::glDebugCallback, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	}
#endif // DEBUG

	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, m_WindowInfo.Width, m_WindowInfo.Height);

	_pCamera = new CCamera(glm::dvec3(0.0), double(m_WindowInfo.Width) / m_WindowInfo.Height);

	if (!_initV()) return false;

	return true;
}

//*********************************************************************
//FUNCTION:
void glt::CApplicationBase::__destroy()
{
	_destroyV();

	_SAFE_DELETE(_pCamera);
	_SAFE_DELETE(_pWindow);

	glfwTerminate();
}