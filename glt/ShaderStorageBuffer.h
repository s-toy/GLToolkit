#pragma once
#include "Export.h"

namespace glt
{
	class GLT_DECLSPEC CShaderStorageBuffer
	{
	public:
		CShaderStorageBuffer(const void* vData, unsigned int vSize, unsigned int vBindPoint);
		~CShaderStorageBuffer();

		void bind() const;
		void unbind() const;

	private:
		unsigned int m_ObjectID = 0;
	};
}