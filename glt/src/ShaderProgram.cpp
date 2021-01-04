#include "ShaderProgram.h"
#include <fstream>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include "Common.h"
#include "Texture.h"
#include "FileLocator.h"
#include "Utility.h"

using namespace glt;

//********************************************************************
//FUNCTION:
CShaderProgram::CShaderProgram()
{
	m_ProgramID = glCreateProgram();
}

//********************************************************************
//FUNCTION:
CShaderProgram::~CShaderProgram()
{
	glDeleteProgram(m_ProgramID);
}

//*********************************************************************************
//FUNCTION:
void CShaderProgram::addShader(const std::string& vShaderName, EShaderType vShaderType)
{
	_ASSERT(!vShaderName.empty());
	std::string ShaderText = __readShaderFile(CFileLocator::getInstance()->locateFile(vShaderName));
	GLuint ShaderID = 0;
	switch (vShaderType)
	{
	case EShaderType::VERTEX_SHADER:	ShaderID = glCreateShader(GL_VERTEX_SHADER); break;
	case EShaderType::FRAGMENT_SHADER:	ShaderID = glCreateShader(GL_FRAGMENT_SHADER); break;
	case EShaderType::GEOMETRY_SHADER:	ShaderID = glCreateShader(GL_GEOMETRY_SHADER); break;
	case EShaderType::COMPUTE_SHADER:	ShaderID = glCreateShader(GL_COMPUTE_SHADER); break;
	default: _ASSERTE(false); break;
	}

	const char* pShaderText = ShaderText.c_str();
	glShaderSource(ShaderID, 1, &pShaderText, nullptr);
	__compileShader(ShaderID);

	glAttachShader(m_ProgramID, ShaderID);
	__linkProgram(m_ProgramID);
}

//*********************************************************************
//FUNCTION:
void CShaderProgram::updateUniform1i(const std::string& vName, int vValue) const
{
	glUniform1i(__getUniformLocation(vName), vValue);
}


//*********************************************************************************
//FUNCTION:
void CShaderProgram::updateUniform1fv(const std::string& vName, unsigned int vCount, float* vValue) const
{
	glUniform1fv(__getUniformLocation(vName), vCount, vValue);
}

//*********************************************************************************
//FUNCTION:
void CShaderProgram::updateUniformTexture(const std::string& vName, const CTexture* vTexture) const
{
	glUniform1i(__getUniformLocation(vName), vTexture->getBindPoint());
}

//*********************************************************************
//FUNCTION:
void CShaderProgram::updateUniform1f(const std::string& vName, float vValue) const
{
	glUniform1f(__getUniformLocation(vName), vValue);
}

//*********************************************************************
//FUNCTION:
void CShaderProgram::updateUniform2f(const std::string& vName, const glm::vec2& vValue) const
{
	glUniform2f(__getUniformLocation(vName), vValue.x, vValue.y);
}

//*********************************************************************
//FUNCTION:
void CShaderProgram::updateUniform3f(const std::string& vName, const glm::vec3& vValue) const
{
	glUniform3f(__getUniformLocation(vName), vValue.x, vValue.y, vValue.z);
}

//*********************************************************************
//FUNCTION:
void CShaderProgram::updateUniform4f(const std::string& vName, const glm::vec4& vValue) const
{
	glUniform4f(__getUniformLocation(vName), vValue.x, vValue.y, vValue.z, vValue.w);
}

//*********************************************************************
//FUNCTION:
void CShaderProgram::updateUniformMat3(const std::string& vName, const glm::mat3& vValue) const
{
	glUniformMatrix3fv(__getUniformLocation(vName), 1, GL_FALSE, &vValue[0][0]);
}

//*********************************************************************
//FUNCTION:
void CShaderProgram::updateUniformMat4(const std::string& vName, const glm::mat4& vValue) const
{
	glUniformMatrix4fv(__getUniformLocation(vName), 1, GL_FALSE, &vValue[0][0]);
}

//*********************************************************************************
//FUNCTION:
static void __getFilePath(const std::string& vFullPath, std::string& voPathWithoutFileName)
{
	size_t Pos = vFullPath.find_last_of("/\\");
	voPathWithoutFileName = vFullPath.substr(0, Pos + 1);
}

//*********************************************************************************
//FUNCTION:
std::string CShaderProgram::__readShaderFile(const std::string& vFileName) const
{
	std::string FullShaderText = "";
	std::ifstream ShaderFile(vFileName);
	_EARLY_RETURN(!ShaderFile.is_open(), format("ERROR: could not open the shader at: %s.", vFileName.c_str()), {});

	static const std::string IncludeIndentifier = "#include";
	static bool IsRecursiveCall = false;

	std::string LineBuffer;
	while (std::getline(ShaderFile, LineBuffer))
	{
		// Look for the new shader include identifier
		if (LineBuffer.find(IncludeIndentifier) != LineBuffer.npos)
		{
			// Remove the include identifier, this will cause the path to remain
			trim(LineBuffer);
			LineBuffer.erase(0, IncludeIndentifier.size());
			ltrim(LineBuffer);
			LineBuffer = LineBuffer.substr(1, LineBuffer.size() - 2);

			// The include path is relative to the current shader file path
			std::string pathOfThisFile;
			__getFilePath(vFileName, pathOfThisFile);
			LineBuffer.insert(0, pathOfThisFile);

			// By using recursion, the new include file can be extracted
			// and inserted at this location in the shader source code
			IsRecursiveCall = true;
			FullShaderText += __readShaderFile(LineBuffer);

			// Do not add this line to the shader source code, as the include
			// path would generate a compilation issue in the final source code
			continue;
		}

		FullShaderText += LineBuffer + '\n';
	}

	// Only add the null terminator at the end of the complete file,
	// essentially skipping recursive function calls this way
	if (!IsRecursiveCall)
		FullShaderText += '\0';

	ShaderFile.close();

	return FullShaderText;
}

//*********************************************************************
//FUNCTION:
GLint CShaderProgram::__getUniformLocation(const std::string& vName) const
{
	if (m_UniformLocCacheMap.find(vName) == m_UniformLocCacheMap.end())
	{
		m_UniformLocCacheMap[vName] = glGetUniformLocation(m_ProgramID, vName.c_str());
#ifdef _DEBUG
		if (m_UniformLocCacheMap[vName] == -1) _OUTPUT_WARNING(format("The Uniform '%s' does not exist or never be used.", vName.c_str()));
#endif
	}

	return m_UniformLocCacheMap[vName];
}

//*********************************************************************************
//FUNCTION:
void CShaderProgram::__compileShader(GLuint& vShader)
{
	glCompileShader(vShader);

	int Success;
	glGetShaderiv(vShader, GL_COMPILE_STATUS, &Success);

	if (!Success)
	{
		GLint LogLength;
		GLchar* pInfoLog = nullptr;
		glGetShaderiv(vShader, GL_INFO_LOG_LENGTH, &LogLength);

		if (LogLength > 0)
		{
			pInfoLog = new GLchar[LogLength];
			glGetShaderInfoLog(vShader, LogLength, &LogLength, pInfoLog);
			fprintf(stderr, "Compile log = '%s' \n", pInfoLog);
			delete[] pInfoLog;
		}
	}
}

//*********************************************************************************
//FUNCTION:
void CShaderProgram::__linkProgram(GLuint& vProgram)
{
	glLinkProgram(vProgram);

	int Success;
	glGetProgramiv(vProgram, GL_LINK_STATUS, &Success);

	if (!Success)
	{
		GLint LogLength;
		glGetProgramiv(vProgram, GL_INFO_LOG_LENGTH, &LogLength);

		if (LogLength > 0)
		{
			auto pInfoLog = new GLchar[LogLength];
			glGetProgramInfoLog(vProgram, LogLength, &LogLength, pInfoLog);
			fprintf(stderr, "Link log = '%s' \n", pInfoLog);
			delete[] pInfoLog;
		}
	}
}
