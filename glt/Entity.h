#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Export.h"

namespace glt
{
	class GLT_DECLSPEC CEntity
	{
	public:
		CEntity();
		virtual ~CEntity();

		const glm::vec3& getPosition() const { return m_Position; }
		const glm::vec3& getScale() const { return m_Scale; }
		const glm::vec4& getRotation() const { return m_Rotation; }

		void setPosition(const glm::vec3& vPosition) { m_Position = vPosition; }
		void setScale(const glm::vec3& vScale) { m_Scale = vScale; }
		void setRotation(float vAngle, glm::vec3 vAxis) { m_Rotation = glm::vec4(vAxis, 0.0); m_Rotation.w = vAngle; };

	private:
		glm::vec3 m_Position = { 0.0, 0.0, 0.0 };
		glm::vec3 m_Scale = { 1.0, 1.0, 1.0 };
		glm::vec4 m_Rotation = { 1.0, 0.0, 0.0, 0.0 };
	};
}