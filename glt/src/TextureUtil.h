#pragma once
#include <vector>
#include <GLAD/glad.h>

namespace glt
{
	namespace util
	{
		GLuint loadCubemap(const std::vector<std::string>& vFaces, bool vGenerateMipMap);

		GLuint setupCubemap(int vWidth, int vHeight, bool vGenerateMipMap);

		GLuint loadTexture(const char *vPath, GLint vWrapMode = GL_CLAMP, GLint vFilterMode = GL_LINEAR, GLenum vFormat = GL_RGB, bool vFlipVertically = false);

		GLuint setupTexture(unsigned int vWidth, unsigned int vHeight, GLint vInternalFormat = GL_RGB32F, GLenum vFormat = GL_RGB, bool vGenerateMipMap = GL_FALSE);
	}
}