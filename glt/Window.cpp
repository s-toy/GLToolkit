#include "Window.h"
#include <algorithm>
#include "Common.h"

using namespace glt;

CWindow::CWindow()
{
}

CWindow::~CWindow()
{
	glfwDestroyWindow(m_pWindow);
}

//***********************************************************************************
//FUNCTION:
bool CWindow::__verifyWindowInfo()
{
	if (m_WindowInfo.MonitorID >= CMonitorManager::getInstance()->getNumMonitors())
	{
		_OUTPUT_WARNING("Invalid [MonitorID] : [MonitorID] must be smaller than the number of monitors.");
		return false;
	}

	m_MonitorInfo = CMonitorManager::getInstance()->getMonitorInfo(m_WindowInfo.MonitorID);

	if (!m_WindowInfo.IsFullScreen && (m_WindowInfo.Width <= 0 || m_WindowInfo.Height <= 0))
	{
		_OUTPUT_WARNING("Window size must be greater than zero.");
		return false;
	}

	if (m_WindowInfo.PosX >= m_MonitorInfo.Resolution.x || m_WindowInfo.PosY >= m_MonitorInfo.Resolution.y)
	{
		_OUTPUT_WARNING("Window position cannot exceed the screen.");
		return false;
	}

	if (m_WindowInfo.IsFullScreen)
	{
		m_WindowInfo.PosX = 0;
		m_WindowInfo.PosY = 0;
		m_WindowInfo.IsResizable = false;
		m_WindowInfo.Width = m_MonitorInfo.Resolution.x;
		m_WindowInfo.Height = m_MonitorInfo.Resolution.y;
	}
	else
	{
		m_WindowInfo.Width = std::min(m_WindowInfo.Width, m_MonitorInfo.Resolution.x - m_WindowInfo.PosX);
		m_WindowInfo.Height = std::min(m_WindowInfo.Height, m_MonitorInfo.Resolution.y - m_WindowInfo.PosY);
	}

	return true;
}

//***********************************************************************************
//FUNCTION:
void CWindow::__createWindow()
{
	m_pWindow = glfwCreateWindow(
		m_WindowInfo.Width,
		m_WindowInfo.Height,
		m_WindowInfo.Title.c_str(),
		nullptr,
		nullptr
	);
	_ASSERTE(m_pWindow);

	int x = m_WindowInfo.PosX + m_MonitorInfo.DisplayArea.x;
	int y = m_WindowInfo.PosY + m_MonitorInfo.DisplayArea.y;
	glfwSetWindowPos(m_pWindow, x, y);
}

//***********************************************************************************
//FUNCTION:
void CWindow::__createFullScreenWindow()
{
	m_pWindow = glfwCreateWindow(
		m_WindowInfo.Width,
		m_WindowInfo.Height,
		m_WindowInfo.Title.c_str(),
		m_MonitorInfo.pMonitor,
		nullptr
	);
	_ASSERTE(m_pWindow);
}

//***********************************************************************************
//FUNCTION:
bool CWindow::createWindow()
{
	return createWindow(m_WindowInfo);
}

//***********************************************************************************
//FUNCTION:
bool CWindow::createFullScreenWindow(uint32_t vMonitorID/* = 0*/)
{
	m_WindowInfo.IsFullScreen = true;
	m_WindowInfo.MonitorID = vMonitorID;
	return createWindow(m_WindowInfo);
}

//***********************************************************************************
//FUNCTION:
bool CWindow::createWindow(const SWindowInfo& vWindowInfo)
{
	if (m_pWindow) return false;

	m_WindowInfo = vWindowInfo;
	if (!__verifyWindowInfo()) return false;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, m_WindowInfo.IsResizable);

#ifdef _DEBUG
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif

	m_WindowInfo.IsFullScreen ? __createFullScreenWindow() : __createWindow();

	glfwMakeContextCurrent(m_pWindow);

	return true;
}