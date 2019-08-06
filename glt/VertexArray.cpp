#include "VertexArray.h"

using namespace glt;

CVertexArray::CVertexArray()
{
	glGenVertexArrays(1, &m_VertexArrayObjectID);
	glBindVertexArray(m_VertexArrayObjectID);
}

CVertexArray::~CVertexArray()
{
	glDeleteVertexArrays(1, &m_VertexArrayObjectID);
}

//*********************************************************************
//FUNCTION:
void CVertexArray::addBuffer(const CVertexBuffer& vBuffer, const CVertexArrayLayout& vLayout)
{
	vBuffer.bind();

	unsigned int Offset = 0;
	for (auto i = 0u; i < vLayout.getNumElements(); ++i)
	{
		auto& Element = vLayout.getElementAt(i);
		glEnableVertexAttribArray(i);
		glVertexAttribPointer(i, Element.Count, Element.Type, Element.Normalized, vLayout.getStride(), (GLvoid*)Offset);
		Offset += Element.Count * SVertexArrayElement::getSizeOfType(Element.Type);
	}
}

//*********************************************************************
//FUNCTION:
void glt::CVertexArray::bind() const
{
	glBindVertexArray(m_VertexArrayObjectID);
}

//*********************************************************************
//FUNCTION:
void glt::CVertexArray::unbind() const
{
	glBindVertexArray(0);
}