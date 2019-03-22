#pragma once
#include <GL/glew.h>

struct SShaderInfo
{
	GLenum Type;
	const char* pFilename;
	GLuint Shader;
};

class CGLShader
{
public:
	bool loadShaders(SShaderInfo* vioShaders);

	GLuint getProgram() const { return m_Program; }
	void useProgram() { glUseProgram(m_Program); }

private:
	GLuint m_Program;
};