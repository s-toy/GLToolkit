#include "Camera.h"
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>
#include "InputManager.h"
#include "Common.h"

using namespace glt;

const float MOUSE_SENSITIVTY = 0.05f;
const float SCROLL_SENSITIVTY = 2.0f;

//********************************************************************
//FUNCTION:
CCamera::CCamera(glm::dvec3 vCameraPos, double vAspect, double vYaw, double vPitch, glm::dvec3 vWorldUp) :m_CameraPos(vCameraPos), m_Aspect(vAspect), m_Yaw(vYaw), m_Pitch(vPitch), m_WorldUp(vWorldUp)
{
	__updateCameraVectors();
	CInputManager::getInstance()->registerCursorCallbackFunc(_CALLBACK_2(CCamera::__cursorCallback, this));
	CInputManager::getInstance()->registerScrollCallbackFunc(_CALLBACK_2(CCamera::__mouseScrollCallback, this));
}

//********************************************************************
//FUNCTION:
CCamera::~CCamera()
{
}

//************************************************************************************
//Function:
void CCamera::__cursorCallback(double vPosX, double vPosY)
{
	if (!CInputManager::getInstance()->getMouseButtonStatus()[GLFW_MOUSE_BUTTON_LEFT]) return;

	std::array<double, 2> CursorOffset = CInputManager::getInstance()->getCursorOffset();

	m_Yaw += CursorOffset[0] * MOUSE_SENSITIVTY;
	m_Pitch += CursorOffset[1] * MOUSE_SENSITIVTY;

	__updateCameraVectors();
}

//************************************************************************************
//Function:
void CCamera::__mouseScrollCallback(double vOffsetX, double vOffsetY)
{
	if (m_Fovy >= 1.0 && m_Fovy <= 45.0) m_Fovy -= vOffsetY * SCROLL_SENSITIVTY;

	if (m_Fovy < 1.0) { m_Fovy = 1.0; }
	else if (m_Fovy > 45.0) { m_Fovy = 45.0; }
}

//************************************************************************************
//Function:
glm::mat4 CCamera::getViewMatrix() const
{
	return glm::lookAt(m_CameraPos, m_CameraPos + m_CameraFront, m_CameraUp);
}

//************************************************************************************
//Function:
glm::mat4 CCamera::getProjectionMatrix() const
{
	return glm::perspective(glm::radians(m_Fovy), m_Aspect, m_Near, m_Far);
}

//************************************************************************************
//Function:
void CCamera::update()
{
	__processKeyboard();
}

//************************************************************************************
//Function:
void CCamera::__updateCameraVectors()
{
	glm::dvec3 Front;
	Front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
	Front.y = sin(glm::radians(m_Pitch));
	Front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));

	m_CameraFront = glm::normalize(Front);
	m_CameraRight = glm::normalize(glm::cross(m_CameraFront, m_WorldUp));
	m_CameraUp = glm::normalize(glm::cross(m_CameraRight, m_CameraFront));
}

//Function:
void CCamera::__processKeyboard()
{
	if (CInputManager::getInstance()->getKeyStatus()[GLFW_KEY_W]) m_CameraPos += m_MoveSpeed * m_CameraFront;
	if (CInputManager::getInstance()->getKeyStatus()[GLFW_KEY_S]) m_CameraPos -= m_MoveSpeed * m_CameraFront;
	if (CInputManager::getInstance()->getKeyStatus()[GLFW_KEY_D]) m_CameraPos += m_MoveSpeed * m_CameraRight;
	if (CInputManager::getInstance()->getKeyStatus()[GLFW_KEY_A]) m_CameraPos -= m_MoveSpeed * m_CameraRight;
	if (CInputManager::getInstance()->getKeyStatus()[GLFW_KEY_Q]) m_CameraPos += m_MoveSpeed * m_CameraUp;
	if (CInputManager::getInstance()->getKeyStatus()[GLFW_KEY_E]) m_CameraPos -= m_MoveSpeed * m_CameraUp;
}