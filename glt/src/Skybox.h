#pragma once
#include <vector>
#include <string>
#include <memory>
#include "Export.h"

namespace glt
{
	class CTextureCube;
	class CShaderProgram;
	class CVertexArray;
	class CVertexBuffer;

	class GLT_DECLSPEC CSkybox
	{
	public:
		CSkybox(const std::vector<std::string>& vFaces);

	protected:
		void _draw(unsigned int vBindPoint) const;

		std::shared_ptr<CShaderProgram> _getShaderProgram() const { return m_pShaderProgram; }

	private:
		std::shared_ptr<CShaderProgram> m_pShaderProgram;
		std::shared_ptr<CTextureCube>	m_pTexture;
		std::shared_ptr<CVertexArray>	m_pVAO;
		std::shared_ptr<CVertexBuffer>	m_pVBO;

		friend class CRenderer;
	};
}