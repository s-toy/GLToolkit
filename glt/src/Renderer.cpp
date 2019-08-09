#include "Renderer.h"
#include "VertexArray.h"
#include "IndexBuffer.h"
#include "Material.h"

using namespace glt;

CRenderer::CRenderer()
{
}

CRenderer::~CRenderer()
{
}

//*********************************************************************
//FUNCTION:
void CRenderer::draw(const CVertexArray& vVertexArray, const CIndexBuffer& vIndexBuffer, const CMaterial& vMaterial) const
{
	vVertexArray.bind();
	vIndexBuffer.bind();
	vMaterial.bind();

	glDrawElements(GL_TRIANGLES, vIndexBuffer.getCount(), GL_UNSIGNED_INT, nullptr);

#ifdef DEBUG
	vVertexArray.unbind();
	vIndexBuffer.unbind();
	vMaterial.unbind();
#endif // DEBUG
}