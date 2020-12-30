#include "Texture.h"
#include "stb_image/stb_image.h"
#include "Common.h"
#include "FileLocator.h"
#include "ShaderProgram.h"

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
void CTexture2D::load(const char* vPath, GLint vWrapMode, GLint vFilterMode, bool vFlipVertically)
{
	_ASSERTE(vPath);
	m_FilePath = CFileLocator::getInstance()->locateFile(vPath);

	stbi_set_flip_vertically_on_load(vFlipVertically);

	int Width, Height, Channels;
	unsigned char* pImageData = stbi_load(m_FilePath.c_str(), &Width, &Height, &Channels, 0);

	glBindTexture(GL_TEXTURE_2D, m_ObjectID);

	switch (Channels)
	{
	case 1:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, Width, Height, 0, GL_RED, GL_UNSIGNED_BYTE, pImageData);
		break;
	case 3:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, pImageData);
		break;
	case 4:
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
void CTexture2D::load16(const char* vPath, GLint vWrapMode, GLint vFilterMode, bool vFlipVertically)
{
	_ASSERTE(vPath);
	m_FilePath = CFileLocator::getInstance()->locateFile(vPath);

	stbi_set_flip_vertically_on_load(vFlipVertically);

	int Width, Height, Channels;
	unsigned short* pImageData = stbi_load_16(m_FilePath.c_str(), &Width, &Height, &Channels, 0);

	glBindTexture(GL_TEXTURE_2D, m_ObjectID);

	switch (Channels)
	{
	case 1:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R16, Width, Height, 0, GL_RED, GL_UNSIGNED_SHORT, pImageData);
		break;
	case 3:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16, Width, Height, 0, GL_RGB, GL_UNSIGNED_SHORT, pImageData);
		break;
	case 4:
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16, Width, Height, 0, GL_RGBA, GL_UNSIGNED_SHORT, pImageData);
		break;
	default:
		break;
	}

	if (!pImageData) _OUTPUT_WARNING("Failed to load texture due to failure of stbi_loadf().");

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
void CTexture2D::createEmpty(unsigned int vWidth, unsigned int vHeight, GLint vInternalFormat, GLint vWrapMode, GLint vFilterMode, bool vGenerateMipMap)
{
	glBindTexture(GL_TEXTURE_2D, m_ObjectID);

	//NOTE:	1.一般情况下，当pixels为nullptr时，Format和Type的取值可以为任意合法的值。
	//		2.特例：当internalformat为GL_DEPTH_COMPONENT*时，format必需为GL_DEPTH_COMPONENT。
	//https://stackoverflow.com/questions/6073707/creating-a-blank-opengl-texture
	//https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glTexImage2D.xhtml
	GLenum Format = GL_RGB, Type = GL_FLOAT;
	if (vInternalFormat == GL_DEPTH_COMPONENT || vInternalFormat == GL_DEPTH_COMPONENT16 || vInternalFormat == GL_DEPTH_COMPONENT24 || vInternalFormat == GL_DEPTH_COMPONENT32 || vInternalFormat == GL_DEPTH_COMPONENT32F)
		Format = GL_DEPTH_COMPONENT;

	glTexImage2D(GL_TEXTURE_2D, 0, vInternalFormat, vWidth, vHeight, 0, Format, Type, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, vWrapMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, vWrapMode);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, vFilterMode);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, vFilterMode);

	if (vGenerateMipMap)
	{
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
	GLubyte* pImageData = nullptr;

	glBindTexture(GL_TEXTURE_CUBE_MAP, m_ObjectID);
	for (GLuint i = 0; i < vFaces.size(); ++i)
	{
		pImageData = stbi_load(CFileLocator::getInstance()->locateFile(vFaces[i]).c_str(), &Width, &Height, &NrComponents, 0);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB8, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, pImageData);
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
	m_Format = vFormat;
	m_Width = vWidth;
	m_Height = vHeight;

	glBindTexture(GL_TEXTURE_2D, m_ObjectID);
	glTexStorage2D(GL_TEXTURE_2D, 1, m_Format, m_Width, m_Height);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindImageTexture(vBindUnit, m_ObjectID, 0, GL_FALSE, 0, GL_READ_WRITE, m_Format);
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
void CImage2DArray::createEmpty(int vWidth, int vHeight, int vDepth, GLenum vFormat, unsigned int vBindUnit)
{
	m_Format = vFormat;
	m_Width = vWidth;
	m_Height = vHeight;

	glBindTexture(GL_TEXTURE_2D_ARRAY, m_ObjectID);
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, m_Format, m_Width, m_Height, vDepth);
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

	glBindImageTexture(vBindUnit, m_ObjectID, 0, GL_FALSE, 0, GL_READ_WRITE, m_Format);
}

//***********************************************************************************************
//FUNCTION:
void CImage2DArray::bindV(unsigned int vBindPoint) const
{

}

//***********************************************************************************************
//FUNCTION:
void CImage2DArray::unbindV() const
{

}
