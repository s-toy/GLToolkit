#include "VertexBuffer.h"

using namespace glt;

CVertexBuffer::CVertexBuffer(const void* vData, unsigned int vSize, unsigned int vUsage)
{
	glGenBuffers(1, &m_BufferID);
	glBindBuffer(GL_ARRAY_BUFFER, m_BufferID);
	glBufferData(GL_ARRAY_BUFFER, vSize, vData, vUsage);
}

CVertexBuffer::~CVertexBuffer()
{
	glDeleteBuffers(1, &m_BufferID);
}

//*********************************************************************
//FUNCTION:
void glt::CVertexBuffer::bind() const
{
	glBindBuffer(GL_ARRAY_BUFFER, m_BufferID);
}

//*********************************************************************
//FUNCTION:
void glt::CVertexBuffer::unbind() const
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}