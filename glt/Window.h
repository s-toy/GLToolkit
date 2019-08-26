#pragma once
#include <utility>
#include <GLFW/glfw3.h>
#include "Common.h"
#include "MonitorManager.h"

namespace glt
{
	class CWindow
	{
	public:
		CWindow();
		virtual ~CWindow();

		bool createWindow();
		bool createWindow(const SWindowInfo& vWindowInfo);
		bool createFullScreenWindow(uint32_t vMonitorID = 0);

		const SWindowInfo& getWindowInfo() const { return m_WindowInfo; }

		GLFWwindow* getGLFWWindow() const { return m_pWindow; }

	private:
		SWindowInfo		m_WindowInfo = {};
		SMonitorInfo	m_MonitorInfo = {};
		GLFWwindow*		m_pWindow = nullptr;

		bool __verifyWindowInfo();
		void __createWindow();
		void __createFullScreenWindow();
	};
}