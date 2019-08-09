#pragma once

namespace glt
{
	class CVertexArray;
	class CIndexBuffer;
	class CMaterial;

	class CRenderer
	{
	public:
		CRenderer();
		~CRenderer();

		void draw(const CVertexArray& vVertexArray, const CIndexBuffer& vIndexBuffer, const CMaterial& vMaterial) const;
	};
}