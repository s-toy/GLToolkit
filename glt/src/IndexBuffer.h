#pragma once
#include <glad/glad.h>
#include "Export.h"

namespace glt
{
	class GLT_DECLSPEC CIndexBuffer
	{
	public:
		CIndexBuffer(const unsigned int* vData, unsigned int vCount, unsigned int vUsage = GL_STATIC_DRAW);
		~CIndexBuffer();

		void bind() const;
		void unbind() const;

		unsigned int getCount() const { return m_Count; }

	private:
		unsigned int m_ObjectID = 0;
		unsigned int m_Count = 0;
	};
}