#include "ApplicationBase.h"
#include "ShaderProgram.h"
#include "Model.h"
#include "Texture.h"
#include "FrameBuffer.h"

using namespace glt;

const int WIN_WIDTH = 1024;
const int WIN_HEIGHT = 576;

class CMyApplication : public CApplicationBase
{
public:
	bool _initV() override
	{
		setDisplayStatusHint();

		m_pGenGbufferShaderProgram = std::make_unique<CShaderProgram>();
		m_pGenGbufferShaderProgram->addShader("shaders/generate_gbuffer_vs.glsl", EShaderType::VERTEX_SHADER);
		m_pGenGbufferShaderProgram->addShader("shaders/generate_gbuffer_fs.glsl", EShaderType::FRAGMENT_SHADER);

		m_pDeferredShadingProgram = std::make_unique<CShaderProgram>();
		m_pDeferredShadingProgram->addShader("shaders/draw_screen_quad_vs.glsl", EShaderType::VERTEX_SHADER);
		m_pDeferredShadingProgram->addShader("shaders/deferred_shading_fs.glsl", EShaderType::FRAGMENT_SHADER);

		m_pModel = std::make_unique<CModel>("../../resource/models/nanosuit/nanosuit.obj");
		m_pModel->setPosition(glm::vec3(0.0f, -1.5f, 0.0f));
		m_pModel->setScale(glm::vec3(0.2f, 0.2f, 0.2f));

		CRenderer::getInstance()->fetchCamera()->setPosition(glm::dvec3(0, 0, 5));

		m_PositionTex = std::make_shared<CTexture2D>();
		m_PositionTex->createEmpty(WIN_WIDTH, WIN_HEIGHT, GL_RGB16F, GL_RGB);
		m_NormalTex = std::make_shared<CTexture2D>();
		m_NormalTex->createEmpty(WIN_WIDTH, WIN_HEIGHT, GL_RGB16F, GL_RGB);
		m_DiffuseTex = std::make_shared<CTexture2D>();
		m_DiffuseTex->createEmpty(WIN_WIDTH, WIN_HEIGHT, GL_RGB8, GL_RGB);
		m_SpecularTex = std::make_shared<CTexture2D>();
		m_SpecularTex->createEmpty(WIN_WIDTH, WIN_HEIGHT, GL_RGB8, GL_RGB);

		m_pOffscreenFrameBuffer = std::make_unique<CFrameBuffer>(WIN_WIDTH, WIN_HEIGHT);
		m_pOffscreenFrameBuffer->set(EAttachment::COLOR0, m_PositionTex);
		m_pOffscreenFrameBuffer->set(EAttachment::COLOR1, m_NormalTex);
		m_pOffscreenFrameBuffer->set(EAttachment::COLOR2, m_DiffuseTex);
		m_pOffscreenFrameBuffer->set(EAttachment::COLOR3, m_SpecularTex);

		return true;
	}

	void _renderV() override
	{
		auto pRenderer = CRenderer::getInstance();

		//generate gbuffer pass
		m_pOffscreenFrameBuffer->bind();
		pRenderer->clear();
		pRenderer->draw(*m_pModel, *m_pGenGbufferShaderProgram);
		m_pOffscreenFrameBuffer->unbind();

		//deferred shading pass
		pRenderer->clear();
		m_PositionTex->bindV(0);
		m_NormalTex->bindV(1);
		m_DiffuseTex->bindV(2);
		m_SpecularTex->bindV(3);

		m_pDeferredShadingProgram->bind();
		m_pDeferredShadingProgram->updateUniformTexture("uPositionTex", m_PositionTex.get());
		m_pDeferredShadingProgram->updateUniformTexture("uNormalTex", m_NormalTex.get());
		m_pDeferredShadingProgram->updateUniformTexture("uDiffuseTex", m_DiffuseTex.get());
		m_pDeferredShadingProgram->updateUniformTexture("uSpecularTex", m_SpecularTex.get());
		pRenderer->drawScreenQuad(*m_pDeferredShadingProgram);
	}

private:
	std::unique_ptr<CFrameBuffer> m_pOffscreenFrameBuffer;

	std::unique_ptr<CShaderProgram> m_pGenGbufferShaderProgram;
	std::unique_ptr<CShaderProgram> m_pDeferredShadingProgram;

	std::shared_ptr<CTexture2D> m_PositionTex;
	std::shared_ptr<CTexture2D> m_NormalTex;
	std::shared_ptr<CTexture2D> m_DiffuseTex;
	std::shared_ptr<CTexture2D> m_SpecularTex;

	std::unique_ptr<CModel> m_pModel;
};

int main()
{
	CMyApplication App;
	if (!App.init(SWindowInfo(WIN_WIDTH, WIN_HEIGHT, "Per-pixel Shading Demo"))) return -1;
	App.run();

	return 0;
}