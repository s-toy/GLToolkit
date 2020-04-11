#include "InputManager.h"

using namespace glt;

namespace glt
{
	std::array<bool, 1024>	CInputManager::m_KeysStatus = { false };
	std::array<bool, 3>		CInputManager::m_MouseButtonStatus = { false };
	std::array<double, 2>   CInputManager::m_CursorPos = { 0.0 };
	std::array<double, 2>   CInputManager::m_CursorOffset = { 0.0 };
	std::array<double, 2>   CInputManager::m_CursorPosLastFrame = { 0.0 };
	std::array<double, 2>   CInputManager::m_ScrollJourney = { 0.0 };
	std::vector<std::function<void(int, int, int)>>        CInputManager::m_MouseButtonCallbackResponseFuncSet;
	std::vector<std::function<void(double, double)>>       CInputManager::m_CursorCallbackResponseFuncSet;
	std::vector<std::function<void(double, double)>>       CInputManager::m_ScrollCallbackResponseFuncSet;
	std::vector<std::function<void(int, int, int, int)>>   CInputManager::m_KeyCallbackResponseFuncSet;
}

//********************************************************************
//FUNCTION:
CInputManager::CInputManager()
{
}

//********************************************************************
//FUNCTION:
CInputManager::~CInputManager()
{
}

//************************************************************************************
//Function:
void CInputManager::init(GLFWwindow* vWindow)
{
	_ASSERTE(vWindow);
	glfwSetKeyCallback(vWindow, __keyCallbackFunc);
	glfwSetMouseButtonCallback(vWindow, __mouseButtonCallbackFunc);
	glfwSetCursorPosCallback(vWindow, __cursorCallbackFunc);
	glfwSetScrollCallback(vWindow, __scrollCallbackFunc);
}

//************************************************************************************
//Function:
void CInputManager::__keyCallbackFunc(GLFWwindow *vWindow, int vKey, int vScancode, int vAction, int vMode)
{
	if (vKey >= 0 && vKey < 1024)
	{
		if (vAction == GLFW_PRESS) m_KeysStatus[vKey] = true;
		else if (vAction == GLFW_RELEASE) m_KeysStatus[vKey] = false;
	}

	if (!m_KeyCallbackResponseFuncSet.empty()) for (auto Func : m_KeyCallbackResponseFuncSet) Func(vKey, vScancode, vAction, vMode);
}

//************************************************************************************
//Function:
void CInputManager::__mouseButtonCallbackFunc(GLFWwindow* vWindow, int vButton, int vAction, int vMods)
{
	if (vButton >= 0 && vButton < 3)
	{
		if (vAction == GLFW_PRESS) m_MouseButtonStatus[vButton] = true;
		else if (vAction == GLFW_RELEASE) m_MouseButtonStatus[vButton] = false;
	}

	if (!m_MouseButtonCallbackResponseFuncSet.empty()) for (auto Func : m_MouseButtonCallbackResponseFuncSet) Func(vButton, vAction, vMods);
}

//************************************************************************************
//Function:
void CInputManager::__cursorCallbackFunc(GLFWwindow* vWindow, double vPosX, double vPosY)
{
	m_CursorPos[0] = vPosX;
	m_CursorPos[1] = vPosY;
	m_CursorOffset[0] = m_CursorPos[0] - m_CursorPosLastFrame[0];
	m_CursorOffset[1] = m_CursorPosLastFrame[1] - m_CursorPos[1];
	m_CursorPosLastFrame = m_CursorPos;

	if (!m_CursorCallbackResponseFuncSet.empty())
	{
		for (auto Func : m_CursorCallbackResponseFuncSet) Func(vPosX, vPosY);
	}
}

//************************************************************************************
//Function:
void CInputManager::__scrollCallbackFunc(GLFWwindow* vWindow, double vOffsetX, double vOffsetY)
{
	m_ScrollJourney[0] += vOffsetX;
	m_ScrollJourney[1] += vOffsetY;

	if (!m_ScrollCallbackResponseFuncSet.empty())
	{
		for (auto Func : m_ScrollCallbackResponseFuncSet) Func(vOffsetX, vOffsetY);
	}
}

//************************************************************************************
//Function:
void CInputManager::registerCursorCallbackFunc(std::function<void(double, double)> vCursorCallbackFunc)
{
	_ASSERTE(vCursorCallbackFunc);
	m_CursorCallbackResponseFuncSet.push_back(vCursorCallbackFunc);
}

//************************************************************************************
//Function:
void CInputManager::registerKeyCallbackFunc(std::function<void(int, int, int, int)> vKeyCallbackFunc)
{
	_ASSERTE(vKeyCallbackFunc);
	m_KeyCallbackResponseFuncSet.push_back(vKeyCallbackFunc);
}

//************************************************************************************
//Function:
void CInputManager::registerMouseButtonCallbackFunc(std::function<void(int, int, int)> vMouseButtonCallbackFunc)
{
	_ASSERTE(vMouseButtonCallbackFunc);
	m_MouseButtonCallbackResponseFuncSet.push_back(vMouseButtonCallbackFunc);
}

//************************************************************************************
//Function:
void CInputManager::registerScrollCallbackFunc(std::function<void(double, double)> vScrollCallbackFunc)
{
	_ASSERTE(vScrollCallbackFunc);
	m_ScrollCallbackResponseFuncSet.push_back(vScrollCallbackFunc);
}