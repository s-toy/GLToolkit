#pragma once
#include "Common.h"

namespace glt
{
	class CVertexArray;
	class CIndexBuffer;
	class CShaderProgram;
	class CModel;

	class CRenderer
	{
	public:
		~CRenderer() = default;
		_SINGLETON(CRenderer);

		bool init();
		void destroy();

		void clear() const;

		void draw(const CVertexArray& vVertexArray, const CIndexBuffer& vIndexBuffer, const CShaderProgram& vShaderProgram) const;
		void draw(const CModel& vModel, const CShaderProgram& vShaderProgram);

	private:
		CRenderer() = default;
		_DISALLOW_COPY_AND_ASSIGN(CRenderer);
	};
}