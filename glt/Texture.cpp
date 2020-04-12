#include "Texture.h"
#include "stb_image/stb_image.h"
#include "Common.h"
#include "FileLocator.h"

using namespace glt;

//***********************************************************************************************
//FUNCTION:
CTexture::CTexture()
{
	glGenTextures(1, &m_ObjectID);
}

//***********************************************************************************************
//FUNCTION:
CTexture::~CTexture()
{
	glDeleteTextures(1, &m_ObjectID);
}

//***********************************************************************************************
//FUNCTION:
void CTexture2D::load(const char *vPath, GLint vWrapMode /*= GL_CLAMP*/, GLint vFilterMode /*= GL_LINEAR*/, GLenum vFormat /*= GL_RGB*/, bool vFlipVertically /*= false*/)
{
	_ASSERTE(vPath);
	m_FilePath =  CFileLocator::getInstance()->locateFile(vPath);

	stbi_set_flip_vertically_on_load(vFlipVertically);

	int Width, Height, Channels;
	unsigned char *pImageData = nullptr;

	glBindTexture(GL_TEXTURE_2D, m_ObjectID);

	switch (vFormat)
	{
	case GL_RED:
		pImageData = stbi_load(m_FilePath.c_str(), &Width, &Height, &Channels, STBI_grey);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, Width, Height, 0, GL_RED, GL_UNSIGNED_BYTE, pImageData);
		break;
	case GL_RGB:
		pImageData = stbi_load(m_FilePath.c_str(), &Width, &Height, &Channels, STBI_rgb);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, pImageData);
		break;
	case GL_RGBA:
		pImageData = stbi_load(m_FilePath.c_str(), &Width, &Height, &Channels, STBI_rgb_alpha);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pImageData);
		break;
	default:
		break;
	}

	if (!pImageData) _OUTPUT_WARNING("Failed to load texture due to failure of stbi_load().");

	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, vWrapMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, vWrapMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, vFilterMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, vFilterMode);

	glBindTexture(GL_TEXTURE_2D, 0);
	stbi_image_free(pImageData);
}

//***********************************************************************************************
//FUNCTION:
void CTexture2D::createEmpty(unsigned int vWidth, unsigned int vHeight, GLint vInternalFormat /*= GL_RGB32F*/, GLenum vFormat /*= GL_RGB*/, bool vGenerateMipMap /*= GL_FALSE*/)
{
	glBindTexture(GL_TEXTURE_2D, m_ObjectID);
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
		glTexParameteri(GL_TEXTURE_2D, GL_AUTO_GENERATE_MIPMAP, GL_TRUE);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	glBindTexture(GL_TEXTURE_2D, 0);
}

//***********************************************************************************************
//FUNCTION:
void CTexture2D::bindV(unsigned int vBindPoint) const
{
	glActiveTexture(GL_TEXTURE0 + vBindPoint);
	glBindTexture(GL_TEXTURE_2D, m_ObjectID);
	m_BindPoint = vBindPoint;
}

//***********************************************************************************************
//FUNCTION:
void CTexture2D::unbindV() const
{
	glActiveTexture(GL_TEXTURE0 + m_BindPoint);
	glBindTexture(GL_TEXTURE_2D, 0);
}

//***********************************************************************************************
//FUNCTION:
void CTextureCube::load(const std::vector<std::string>& vFaces, bool vGenerateMipMap)
{
	int Width, Height, NrComponents;
	float *pImageData = nullptr;

	glBindTexture(GL_TEXTURE_CUBE_MAP, m_ObjectID);
	for (GLuint i = 0; i < vFaces.size(); ++i)
	{
		pImageData = stbi_loadf(CFileLocator::getInstance()->locateFile(vFaces[i]).c_str(), &Width, &Height, &NrComponents, 0);
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
}

//***********************************************************************************************
//FUNCTION:
void CTextureCube::createEmpty(int vWidth, int vHeight, bool vGenerateMipMap)
{
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_ObjectID);
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
}

//***********************************************************************************************
//FUNCTION:
void CTextureCube::bindV(unsigned int vBindPoint) const
{
	glActiveTexture(GL_TEXTURE0 + vBindPoint);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_ObjectID);
	m_BindPoint = vBindPoint;
}

//***********************************************************************************************
//FUNCTION:
void CTextureCube::unbindV() const
{
	glActiveTexture(GL_TEXTURE0 + m_BindPoint);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

//***********************************************************************************************
//FUNCTION:
void CImage2D::createEmpty(int vWidth, int vHeight, GLenum vFormat, unsigned int vBindUnit)
{
	glBindTexture(GL_TEXTURE_2D, m_ObjectID);
	glTexStorage2D(GL_TEXTURE_2D, 1, vFormat, vWidth, vHeight);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindImageTexture(vBindUnit, m_ObjectID, 0, GL_FALSE, 0, GL_READ_WRITE, vFormat);
}

//***********************************************************************************************
//FUNCTION:
void CImage2D::bindV(unsigned int vBindPoint) const
{

}

//***********************************************************************************************
//FUNCTION:
void CImage2D::unbindV() const
{

}

//***********************************************************************************************
//FUNCTION:
void CImage2D::clear()
{
	glBindTexture(GL_TEXTURE_2D, m_ObjectID);
	std::vector<GLuint> emptyData(1024 * 576, 0);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1024, 576, GL_RED, GL_UNSIGNED_INT, &emptyData[0]);
}