#pragma once
#include "Common.h"
#include "Camera.h"

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

		void update();

		void draw(const CVertexArray& vVertexArray, const CIndexBuffer& vIndexBuffer, const CShaderProgram& vShaderProgram) const;
		void draw(const CModel& vModel, const CShaderProgram& vShaderProgram);

		CCamera* fetchCamera() const { return m_pCamera; }

	private:
		CRenderer() = default;
		_DISALLOW_COPY_AND_ASSIGN(CRenderer);

		void __updateShaderUniform(const CShaderProgram& vShaderProgram) const;

		CCamera* m_pCamera = nullptr;
	};
}