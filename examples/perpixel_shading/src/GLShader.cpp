#include "GLShader.h"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

//**********************************************************************************************
//FUNCTION:
static const GLchar* readShader(const char* vFilename)
{
	FILE* pInfile = fopen(vFilename, "rb");
	if (!pInfile)
	{
		std::cerr << "Unable to open file '" << vFilename << "'" << std::endl;
		return nullptr;
	}

	fseek(pInfile, 0, SEEK_END);
	int FileLen = ftell(pInfile);
	fseek(pInfile, 0, SEEK_SET);
	GLchar* pSource = new GLchar[FileLen + 1];
	fread(pSource, 1, FileLen, pInfile);
	fclose(pInfile);
	pSource[FileLen] = 0;

	return const_cast<const GLchar*>(pSource);
}

//**********************************************************************************************
//FUNCTION:
bool CGLShader::loadShaders(SShaderInfo* vioShaders)
{
	if (NULL == vioShaders) { return false; }

	GLuint ShaderProgram = glCreateProgram();
	SShaderInfo* pEntry = vioShaders;
	while (pEntry->Type != GL_NONE)
	{
		GLuint ShaderObj = glCreateShader(pEntry->Type);
		pEntry->Shader = ShaderObj;
		const GLchar* pSource = readShader(pEntry->pFilename);
		if (NULL == pSource)
		{
			for (pEntry = vioShaders; pEntry->Type != GL_NONE; ++pEntry)
			{
				glDeleteShader(pEntry->Shader);
				pEntry->Shader = 0;
			}
			return false;
		}

		glShaderSource(ShaderObj, 1, &pSource, NULL);
		delete[] pSource;
		glCompileShader(ShaderObj);
		GLint Compiled;
		glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &Compiled);
		if (!Compiled)
		{
			GLsizei Len;
			glGetShaderiv(ShaderObj, GL_INFO_LOG_LENGTH, &Len);
			GLchar* LogInfo = new GLchar[Len + 1];
			glGetShaderInfoLog(ShaderObj, Len, &Len, LogInfo);
			std::cerr << "Shader compilation failed: " << LogInfo << std::endl;
			delete[] LogInfo;
			return false;
		}
		glAttachShader(ShaderProgram, ShaderObj);

		++pEntry;
	}

	glLinkProgram(ShaderProgram);
	GLint Linked;
	glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Linked);
	if (!Linked)
	{
		GLsizei Len;
		glGetProgramiv(ShaderProgram, GL_INFO_LOG_LENGTH, &Len);

		GLchar* LogInfo = new GLchar[Len + 1];
		glGetProgramInfoLog(ShaderProgram, Len, &Len, LogInfo);
		std::cerr << "Shader linking failed: " << LogInfo << std::endl;
		delete[] LogInfo;

		for (pEntry = vioShaders; pEntry->Type != GL_NONE; ++pEntry)
		{
			glDeleteShader(pEntry->Shader);
			pEntry->Shader = 0;
		}
		return false;
	}

	m_Program = ShaderProgram;
	return true;
}