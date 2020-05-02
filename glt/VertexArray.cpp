#include "VertexArray.h"

using namespace glt;

//********************************************************************
//FUNCTION:
CVertexArray::CVertexArray()
{
	glGenVertexArrays(1, &m_ObjectID);
	glBindVertexArray(m_ObjectID);
}

//********************************************************************
//FUNCTION:
CVertexArray::~CVertexArray()
{
	glDeleteVertexArrays(1, &m_ObjectID);
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

		if (Element.Type == GL_INT || Element.Type == GL_UNSIGNED_INT) //TODO:
			glVertexAttribIPointer(i, Element.Count, Element.Type, vLayout.getStride(), (void*)Offset);
		else 
			glVertexAttribPointer(i, Element.Count, Element.Type, Element.Normalized, vLayout.getStride(), (void*)Offset);

		Offset += Element.Count * SVertexArrayElement::getSizeOfType(Element.Type);
	}
}

//*********************************************************************
//FUNCTION:
void glt::CVertexArray::bind() const
{
	glBindVertexArray(m_ObjectID);
}

//*********************************************************************
//FUNCTION:
void glt::CVertexArray::unbind() const
{
	glBindVertexArray(0);
}