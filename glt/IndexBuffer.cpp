#include "IndexBuffer.h"

using namespace glt;

CIndexBuffer::CIndexBuffer(const unsigned int* vData, unsigned int vCount, unsigned int vUsage) : m_Count(vCount)
{
	glGenBuffers(1, &m_BufferID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_BufferID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, vCount * sizeof(unsigned int), vData, vUsage);
}

CIndexBuffer::~CIndexBuffer()
{
	glDeleteBuffers(1, &m_BufferID);
}

//*********************************************************************
//FUNCTION:
void glt::CIndexBuffer::bind() const
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_BufferID);
}

//*********************************************************************
//FUNCTION:
void glt::CIndexBuffer::unbind() const
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}