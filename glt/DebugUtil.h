#pragma once
#include <glad/glad.h>

#ifdef _DEBUG
void gltDebugCallback(GLenum vSource, GLenum vType, GLuint vID, GLenum vSeverity, GLsizei vLength, const GLchar* vMessage, const void* vUserParam);
#endif // DEBUG

