#pragma once

namespace glt
{
	class CShaderProgram;

	class CMaterial
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