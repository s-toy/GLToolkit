#pragma once
#include <string>
#include <unordered_map>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include "Export.h"

namespace glt
{
	enum class EShaderType : char
	{
		VERTEX_SHADER = 0,
		FRAGMENT_SHADER,
		GEOMETRY_SHADER,
		COMPUTE_SHADER
	};

	class GLT_DECLSPEC CShaderProgram
	{
	public:
		CShaderProgram();
		~CShaderProgram();

		void bind() const { glUseProgram(m_ProgramID); }
		void unbind() const { glUseProgram(0); }

		void addShader(const std::string& vShaderName, EShaderType vShaderType);

		unsigned int getProgramID() const { return m_ProgramID; }

		void updateUniform1i(const std::string& vName, int vValue) const;

		void updateUniform1f(const std::string& vName, float vValue) const;
		void updateUniform2f(const std::string& vName, const glm::vec2& vValue) const;
		void updateUniform3f(const std::string& vName, const glm::vec3& vValue) const;
		void updateUniform4f(const std::string& vName, const glm::vec4& vValue) const;

		void updateUniformMat3(const std::string& vName, const glm::mat3& vValue) const;
		void updateUniformMat4(const std::string& vName, const glm::mat4& vValue) const;

	private:
		GLuint m_ProgramID;

		mutable std::unordered_map<std::string, GLint> m_UniformLocCacheMap;

		const GLchar* const __readShaderFile(const std::string& vFileName);
		GLint __getUniformLocation(const std::string& vName) const;
		void __compileShader(GLuint& vShader);
		void __linkProgram(GLuint& vProgram);
	};
}