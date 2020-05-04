#include "AtomicCounterBuffer.h"
#include <glad/glad.h>

using namespace glt;

//***************************************************************************
//FUNCTION:
CAtomicCounterBuffer::CAtomicCounterBuffer(unsigned int vBindUnit) : m_BindUnit(vBindUnit)
{
	//init the atomic counter
	glGenBuffers(1, &m_ObjectID);
	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, m_BindUnit, m_ObjectID);

	GLuint zero = 0;
	glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), &zero, GL_DYNAMIC_DRAW);
}

//***************************************************************************
//FUNCTION:
CAtomicCounterBuffer::~CAtomicCounterBuffer()
{
	glDeleteBuffers(1, &m_ObjectID);
}

//***************************************************************************
//FUNCTION:
void CAtomicCounterBuffer::reset()
{
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, m_ObjectID);
	GLuint zero = 0;
	glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &zero);
}