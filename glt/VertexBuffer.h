#pragma once
#include <glad/glad.h>
#include "Export.h"

namespace glt
{
	class GLT_DECLSPEC CVertexBuffer
	{
	public:
		CVertexBuffer(const void* vData, unsigned int vSize, unsigned int vUsage = GL_STATIC_DRAW);
		~CVertexBuffer();

		void bind() const;
		void unbind() const;

	private:
		unsigned int m_BufferID = 0;
	};
}