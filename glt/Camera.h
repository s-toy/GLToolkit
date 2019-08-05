#pragma once
#include <GLM/glm.hpp>

namespace glt
{
	class CCamera
	{
	public:
		CCamera(glm::dvec3 vCameraPos = glm::dvec3(0.0, 0.0, 0.0), double vAspect = 1.0, double vYaw = -90.0, double vPitch = 0.0, glm::dvec3 vWorldUp = glm::dvec3(0.0, 1.0, 0.0));
		~CCamera();

		void update();

		glm::mat4	getViewMatrix() const;
		glm::mat4	getProjectionMatrix() const;
		glm::dvec3	getPosition() const { return m_CameraPos; }
		double		getFovy() const { return m_Fovy; }
		double		getAspect() const { return m_Aspect; }

		void setPosition(glm::dvec3 vCameraPos) { m_CameraPos = vCameraPos; }
		void setMoveSpeed(double vMoveSpeed)	{ m_MoveSpeed = vMoveSpeed; }
		void setFarPlane(double vFarPlane)		{ m_Far = vFarPlane; }
		void setNearPlane(double vNearPlane)	{ m_Near = vNearPlane; }
		void setFovy(double vFovy)				{ m_Fovy = vFovy; }
		void setAspect(double vAspect)			{ m_Aspect = vAspect; }

	private:
		void __cursorCallback(double vPosX, double vPosY);
		void __mouseScrollCallback(double vOffsetX, double vOffsetY);

		void __updateCameraVectors();
		void __processKeyboard();

		glm::mat4 m_ViewMatrix;
		glm::mat4 m_ProjectionMatrix;

		glm::dvec3 m_CameraPos;
		glm::dvec3 m_CameraUp;
		glm::dvec3 m_CameraFront;
		glm::dvec3 m_CameraRight;
		glm::dvec3 m_WorldUp;

		double m_MoveSpeed = 0.01;
		double m_Pitch = 0.0;
		double m_Yaw = 0.0;

		double m_Fovy = 45.0;
		double m_Aspect = 1.0;
		double m_Near = 0.1;
		double m_Far = 100.0;

		bool m_IsEnableCursor = true;
	};
}