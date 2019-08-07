#include "TextureUtil.h"
#include "../external/stb_image.h"
#include "Common.h"

using namespace glt;

//**********************************************************************************************
//FUNCTION:
GLuint util::loadCubemap(const std::vector<std::string>& vFaces, bool vGenerateMipMap)
{
	GLuint TextureID;
	glGenTextures(1, &TextureID);

	int Width, Height, NrComponents;
	float *pImageData = nullptr;

	glBindTexture(GL_TEXTURE_CUBE_MAP, TextureID);
	for (GLuint i = 0; i < vFaces.size(); ++i)
	{
		pImageData = stbi_loadf(vFaces[i].c_str(), &Width, &Height, &NrComponents, 0);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, Width, Height, 0, GL_RGB, GL_FLOAT, pImageData);
		stbi_image_free(pImageData);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	if (vGenerateMipMap)
	{
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	return TextureID;
}

//**********************************************************************************************
//FUNCTION:
GLuint util::setupCubemap(int vWidth, int vHeight, bool vGenerateMipMap)
{
	GLuint Cubemap;
	glGenTextures(1, &Cubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, Cubemap);
	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB32F, vWidth, vHeight, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	if (vGenerateMipMap)
	{
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	return Cubemap;
}

//**********************************************************************************************
//FUNCTION:
GLuint util::loadTexture(const char *vPath, GLint vWrapMode, GLint vFilterMode, GLenum vFormat, bool vFlipVertically)
{
	stbi_set_flip_vertically_on_load(vFlipVertically);

	int Width, Height, Channels;
	unsigned char *pImageData = nullptr;

	GLuint TextureID;
	glGenTextures(1, &TextureID);
	glBindTexture(GL_TEXTURE_2D, TextureID);

	switch (vFormat)
	{
	case GL_RED:
		pImageData = stbi_load(vPath, &Width, &Height, &Channels, STBI_grey);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, Width, Height, 0, GL_RED, GL_UNSIGNED_BYTE, pImageData);
		break;
	case GL_RGB:
		pImageData = stbi_load(vPath, &Width, &Height, &Channels, STBI_rgb);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, pImageData);
		break;
	case GL_RGBA:
		pImageData = stbi_load(vPath, &Width, &Height, &Channels, STBI_rgb_alpha);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pImageData);
		break;
	default:
		break;
	}

	_EARLY_RETURN(!pImageData, "Failed to load texture due to failure of stbi_load().", 0);

	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, vWrapMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, vWrapMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, vFilterMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, vFilterMode);

	glBindTexture(GL_TEXTURE_2D, 0);
	stbi_image_free(pImageData);

	return TextureID;
}

//**********************************************************************************************
//FUNCTION:
GLuint util::setupTexture(unsigned int vWidth, unsigned int vHeight, GLint vInternalFormat, GLenum vFormat, bool vGenerateMipMap)
{
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, vInternalFormat, vWidth, vHeight, 0, vFormat, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	if (vGenerateMipMap == false)
	{
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	glBindTexture(GL_TEXTURE_2D, 0);

	return texture;
}