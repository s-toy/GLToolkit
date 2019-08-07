#pragma once
#include <glad/glad.h>

namespace glt
{
	class CIndexBuffer
	{
	public:
		CIndexBuffer(const unsigned int* vData, unsigned int vCount, unsigned int vUsage = GL_STATIC_DRAW);
		~CIndexBuffer();

		void bind() const;
		void unbind() const;

		unsigned int getCount() const { return m_Count; }

	private:
		unsigned int m_BufferID = 0;
		unsigned int m_Count = 0;
	};
}