#include "ShaderProgram.h"
#include <fstream>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include "Common.h"
#include "Texture.h"
#include "FileLocator.h"

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
	const GLchar* pShaderText = __readShaderFile(CFileLocator::getInstance()->locateFile(vShaderName));
	_ASSERT(pShaderText);

	GLuint ShaderID = 0;
	switch (vShaderType)
	{
	case EShaderType::VERTEX_SHADER:
		ShaderID = glCreateShader(GL_VERTEX_SHADER);
		break;
	case EShaderType::FRAGMENT_SHADER:
		ShaderID = glCreateShader(GL_FRAGMENT_SHADER);
		break;
	case EShaderType::GEOMETRY_SHADER:
		ShaderID = glCreateShader(GL_GEOMETRY_SHADER);
		break;
	case EShaderType::COMPUTE_SHADER:
		ShaderID = glCreateShader(GL_COMPUTE_SHADER);
		break;
	default:
		break;
	}

	glShaderSource(ShaderID, 1, &pShaderText, nullptr);
	__compileShader(ShaderID);

	glAttachShader(m_ProgramID, ShaderID);
	__linkProgram(m_ProgramID);

	delete pShaderText;
}

//*********************************************************************
//FUNCTION:
void CShaderProgram::updateUniform1i(const std::string& vName, int vValue) const
{
	glUniform1i(__getUniformLocation(vName), vValue);
}

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
const GLchar* const CShaderProgram::__readShaderFile(const std::string& vFileName)
{
	_ASSERT(vFileName.size() != 0);
	GLchar* pShaderText = nullptr;
	std::filebuf* pFileBuffer = nullptr;
	unsigned int FileSize = 0;

	std::ifstream FileIn;
	FileIn.open(vFileName, std::ios::binary);

	if (FileIn.fail())
	{
		std::cout << "Fail to open " << std::endl;
		return nullptr;
	}

	pFileBuffer = FileIn.rdbuf();
	FileSize = (unsigned int)pFileBuffer->pubseekoff(0, std::ios::end, std::ios::in);
	pFileBuffer->pubseekpos(0, std::ios::in);

	pShaderText = new GLchar[FileSize + 1];
	pFileBuffer->sgetn(pShaderText, FileSize);
	pShaderText[FileSize] = '\0';
	FileIn.close();

	return pShaderText;
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