#include "ShaderStorageBuffer.h"
#include <glad/glad.h>

using namespace glt;

//********************************************************************
//FUNCTION:
CShaderStorageBuffer::CShaderStorageBuffer(const void* vData, unsigned int vSize, unsigned int vBindPoint)
{
	glGenBuffers(1, &m_ObjectID);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, vBindPoint, m_ObjectID);
	glBufferData(GL_SHADER_STORAGE_BUFFER, vSize, vData, GL_DYNAMIC_COPY);
}

//********************************************************************
//FUNCTION:
CShaderStorageBuffer::~CShaderStorageBuffer()
{
	glDeleteBuffers(1, &m_ObjectID);
}

//********************************************************************
//FUNCTION:
void CShaderStorageBuffer::bind() const
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ObjectID);
}

//********************************************************************
//FUNCTION:
void CShaderStorageBuffer::unbind() const
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}