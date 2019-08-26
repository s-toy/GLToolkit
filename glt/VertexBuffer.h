#pragma once
#include <glad/glad.h>

namespace glt
{
	class CVertexBuffer
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