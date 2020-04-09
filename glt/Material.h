#pragma once
#include "Export.h"

namespace glt
{
	class CShaderProgram;

	class GLT_DECLSPEC CMaterial
	{
	public:
		CMaterial();
		~CMaterial();

		void bind() const;
		void unbind() const;

	private:
		//CShaderProgram*
	};
}