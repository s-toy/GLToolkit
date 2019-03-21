#pragma once

#include <GL/glew.h>

typedef struct {
	GLenum Type;
	const char* pFilename;
	GLuint Shader;
} SShaderInfo;

class CShader {
public:
	bool loadShaders(SShaderInfo* vioShaders);

	GLuint getProgram();

	void useProgram();

private:
	GLuint m_Program;
};