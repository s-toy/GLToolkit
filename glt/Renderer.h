#pragma once
#include <memory>
#include "Common.h"
#include "Camera.h"
#include "Export.h"

namespace glt
{
	class CVertexArray;
	class CVertexBuffer;
	class CIndexBuffer;
	class CShaderProgram;
	class CModel;

	class GLT_DECLSPEC CRenderer
	{
	public:
		~CRenderer() = default;
		_SINGLETON(CRenderer);

		bool init();
		void destroy();

		void clear() const;

		void update();

		void draw(const CVertexArray& vVertexArray, const CIndexBuffer& vIndexBuffer, const CShaderProgram& vShaderProgram) const;
		void draw(const CModel& vModel, const CShaderProgram& vShaderProgram);
		void drawScreenQuad(const CShaderProgram& vShaderProgram);

		CCamera* fetchCamera() const { return m_pCamera; }

	private:
		CRenderer() = default;
		_DISALLOW_COPY_AND_ASSIGN(CRenderer);

		void __updateShaderUniform(const CShaderProgram& vShaderProgram) const;
		void __initFullScreenQuad();

		CCamera* m_pCamera = nullptr;

		std::shared_ptr<CVertexArray>	m_FullScreenQuadVAO;
		std::shared_ptr<CVertexBuffer>	m_FullScreenQuadVBO;
	};
}