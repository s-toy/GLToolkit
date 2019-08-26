#pragma once
#include "VertexBuffer.h"
#include "VertexArrayLayout.h"

namespace glt
{
	class CVertexArray
	{
	public:
		CVertexArray();
		~CVertexArray();

		void addBuffer(const CVertexBuffer& vBuffer, const CVertexArrayLayout& vLayout);
		void bind() const;
		void unbind() const;

	private:
		unsigned int m_ObjectID = 0;
	};
}