#pragma once
#include <GLFW/glfw3.h>
#include <functional>
#include <array>
#include <vector>
#include "Common.h"

namespace glt
{
	class CInputManager
	{
	public:
		~CInputManager();

		_SINGLETON(CInputManager);

		void init(GLFWwindow* vWindow);

		void registerKeyCallbackFunc(std::function<void(int, int, int, int)> vKeyCallbackFunc);
		void registerCursorCallbackFunc(std::function<void(double, double)> vCursorCallbackFunc);
		void registerScrollCallbackFunc(std::function<void(double, double)> vScrollCallbackFunc);
		void registerMouseButtonCallbackFunc(std::function<void(int, int, int)> vMouseButtonCallbackFunc);

		const std::array<bool, 1024>&	getKeyStatus() const { return m_KeysStatus; }
		const std::array<bool, 3>&		getMouseButtonStatus() const { return m_MouseButtonStatus; }
		const std::array<double, 2>&    getCursorPos() const { return m_CursorPos; }
		const std::array<double, 2>&    getCursorOffset() const { return m_CursorOffset; }
		const std::array<double, 2>&    getScrollJourney() const { return m_ScrollJourney; }

	protected:
		CInputManager();
		_DISALLOW_COPY_AND_ASSIGN(CInputManager);

	private:
		static std::array<bool, 1024>	m_KeysStatus;
		static std::array<bool, 3>		m_MouseButtonStatus;
		static std::array<double, 2>    m_CursorPos;
		static std::array<double, 2>    m_CursorPosLastFrame;
		static std::array<double, 2>    m_CursorOffset;
		static std::array<double, 2>    m_ScrollJourney; ////滚轮从一开始累计滑动的路程

		static std::vector<std::function<void(int, int, int)>>			m_MouseButtonCallbackResponseFuncSet;
		static std::vector<std::function<void(double, double)>>         m_CursorCallbackResponseFuncSet;
		static std::vector<std::function<void(double, double)>>         m_ScrollCallbackResponseFuncSet;
		static std::vector<std::function<void(int, int, int, int)>>		m_KeyCallbackResponseFuncSet;

		static void __keyCallbackFunc(GLFWwindow *vWindow, int vKey, int vScancode, int vAction, int vMode);
		static void __cursorCallbackFunc(GLFWwindow* vWindow, double vPosX, double vPosY);
		static void __scrollCallbackFunc(GLFWwindow* vWindow, double vOffsetX, double vOffsetY);
		static void __mouseButtonCallbackFunc(GLFWwindow* vWindow, int vButton, int vAction, int vMods);
	};
}