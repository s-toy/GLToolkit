#include "ApplicationBase.h"
#include <GLFW/glfw3.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include "Window.h"
#include "InputManager.h"

using namespace glt;

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
bool glt::CApplicationBase::init(const SWindowInfo& vWindowInfo)
{
	_EARLY_RETURN(m_IsInitialized, "The application has already been initialzed. ", true);
	_EARLY_RETURN(!glfwInit(), "Failed to initialize glfw.", false);

#ifdef _DEBUG
	glfwSetErrorCallback(__glfwErrorCallback);
#endif

	m_WindowInfo = vWindowInfo;

	_pWindow = new CWindow;
	_EARLY_RETURN(!_pWindow->createWindow(m_WindowInfo), "Failed to create window.", false);

	_EARLY_RETURN(!CRenderer::getInstance()->init(), "Failed to initialize renderer.", false);
	CRenderer::getInstance()->fetchCamera()->setAspect((double)m_WindowInfo.Width / m_WindowInfo.Height);

	CInputManager::getInstance()->init(_pWindow->getGLFWWindow());

	_EARLY_RETURN(!__initIMGUI(), "Failed to initailize ImGUI.", false);

	if (!_initV()) return false;

	_OUTPUT_EVENT("Succeed to init application.");

	m_IsInitialized = true;

	return true;
}

//*********************************************************************
//FUNCTION:
void glt::CApplicationBase::run()
{
	if (!m_IsInitialized) { _OUTPUT_WARNING("The Application has not been initialzed."); return; }

	try
	{
		while (!glfwWindowShouldClose(_pWindow->getGLFWWindow()))
		{
			_updateV();
			CRenderer::getInstance()->update();

			_renderV();
			__renderGUI();

			glfwSwapBuffers(_pWindow->getGLFWWindow());
			glfwPollEvents();
		}

		__destroy();

		_OUTPUT_EVENT("Succeed to end application.");
	}
	catch (const std::exception& e)
	{
		_OUTPUT_WARNING(e.what());
		exit(EXIT_FAILURE);
	}
}

//***********************************************************************************************
//FUNCTION:
bool glt::CApplicationBase::__initIMGUI()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	if (!ImGui_ImplGlfw_InitForOpenGL(_pWindow->getGLFWWindow(), true)) return false;
	ImGui_ImplOpenGL3_Init("#version 430");

	return true;
}

//*********************************************************************
//FUNCTION:
void glt::CApplicationBase::__renderGUI()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	if (m_DisplayAppStatus)
	{
		ImGui::Begin("Application Status");
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
	}

	_onGuiV();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

//*********************************************************************
//FUNCTION:
void glt::CApplicationBase::__destroy()
{
	_SAFE_DELETE(_pWindow);

	CRenderer::getInstance()->destroy();

	glfwTerminate();
}