#pragma once
#include <memory>
#include <vector>
#include <glad/glad.h>
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
	class CSkybox;

	class GLT_DECLSPEC CRenderer
	{
	public:
		~CRenderer() = default;
		_SINGLETON(CRenderer);

		bool init();
		void destroy();

		void clear(GLbitfield vBuffers = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) const;
		void setClearColor(float vRed, float vGreen, float vBlue, float vAlpha) const;
		void clearBuffer(GLuint vDrawBuffer, const GLfloat* vData) const;
		void update();

		void enableCullFace(bool vEnable) const;
		void setDepthMask(bool vFlag) const;

		void enableBlend(bool vEnable) const;  //TODO: 无意义的封装
		void setBlendFunc(GLenum vSrc, GLenum vDst, int vBufferIndex = -1) const; //

		void memoryBarrier(GLbitfield vBarriers) const;

		void draw(const CVertexArray& vVertexArray, const CIndexBuffer& vIndexBuffer, const CShaderProgram& vShaderProgram) const;
		void draw(const CModel& vModel, const CShaderProgram& vShaderProgram);
		void draw(const std::vector<std::shared_ptr<CModel>>& vModels, const CShaderProgram& vShaderProgram);
		void drawScreenQuad(const CShaderProgram& vShaderProgram);
		void drawSkybox(const CSkybox& vSkybox, unsigned int vBindPoint);

		CCamera* fetchCamera() const { return m_pCamera; }

	protected:
		void _setTime(float vTime) { m_Time = vTime; }

	private:
		CRenderer() = default;
		_DISALLOW_COPY_AND_ASSIGN(CRenderer);

		void __drawSingleModel(const CModel& vModel, const CShaderProgram& vShaderProgram);
		void __updateShaderUniform(const CShaderProgram& vShaderProgram) const;
		void __initFullScreenQuad();

		CCamera* m_pCamera = nullptr;
		float m_Time = 0.0f;

		std::shared_ptr<CVertexArray>	m_FullScreenQuadVAO;
		std::shared_ptr<CVertexBuffer>	m_FullScreenQuadVBO;

		friend class CApplicationBase;
	};
}