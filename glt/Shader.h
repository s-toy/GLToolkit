#pragma once
#include <string>
#include <unordered_map>
#include <gl/glew.h>
#include <glm/glm.hpp>

enum class EShaderType : char
{
	VERTEX_SHADER = 0,
	FRAGMENT_SHADER,
	GEOMETRY_SHADER,
	COMPUTE_SHADER
};

class CShader
{
public:
	CShader();
	~CShader();

	void addShader(const std::string& vShaderName, EShaderType vShaderType);
	void enableShader() const { glUseProgram(m_ProgramID); }
	void disableShader() const { glUseProgram(0); }
	unsigned int getProgramID() const { return m_ProgramID; }

	void setUniform1i(const std::string& vName, int vValue);

	void setUniform1f(const std::string& vName, float vValue);
	void setUniform2f(const std::string& vName, const glm::vec2& vValue);
	void setUniform3f(const std::string& vName, const glm::vec3& vValue);
	void setUniform4f(const std::string& vName, const glm::vec4& vValue);

	void setUniformMat3(const std::string& vName, const glm::mat3& vValue);
	void setUniformMat4(const std::string& vName, const glm::mat4& vValue);

private:
	GLuint m_ProgramID;

	mutable std::unordered_map<std::string, GLint> m_UniformLocCacheMap;

	const GLchar* const __readShaderFile(const std::string& vFileName);
	GLint __getUniformLocation(const std::string& vName) const;
	void __compileShader(GLuint& vShader);
	void __linkProgram(GLuint& vProgram);
	void __printUniformWarningInfo(const std::string& vUniform);
};