#include <memory>
#include <vector>
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

		m_pOpaqueShaderProgram = std::make_unique<CShaderProgram>();
		m_pOpaqueShaderProgram->addShader("shaders/opaque_shading_vs.glsl", EShaderType::VERTEX_SHADER);
		m_pOpaqueShaderProgram->addShader("shaders/opaque_shading_fs.glsl", EShaderType::FRAGMENT_SHADER);

		m_pColorBlendingProgram = std::make_unique<CShaderProgram>();
		m_pColorBlendingProgram->addShader("shaders/draw_screen_quad_vs.glsl", EShaderType::VERTEX_SHADER);
		m_pColorBlendingProgram->addShader("shaders/color_blending_fs.glsl", EShaderType::FRAGMENT_SHADER);

		m_OpaqueModels.push_back(std::make_unique<CModel>("../../resource/models/nanosuit/nanosuit.obj"));
		m_OpaqueModels[0]->setPosition(glm::vec3(0.0f, -1.5f, 0.0f));
		m_OpaqueModels[0]->setScale(glm::vec3(0.2f, 0.2f, 0.2f));

		m_pOpaqueColorTexture = std::make_shared<CTexture2D>();
		m_pOpaqueColorTexture->createEmpty(WIN_WIDTH, WIN_HEIGHT, GL_RGB8, GL_RGB);

		m_pOpaqueFrameBuffer = std::make_unique<CFrameBuffer>(WIN_WIDTH, WIN_HEIGHT);
		m_pOpaqueFrameBuffer->set(EAttachment::COLOR0, m_pOpaqueColorTexture);

		CRenderer::getInstance()->fetchCamera()->setPosition(glm::dvec3(0, 0, 5));

		return true;
	}

	void _renderV() override
	{
		//draw opaque objects
		m_pOpaqueFrameBuffer->bind();
		CRenderer::getInstance()->clear();
		CRenderer::getInstance()->draw(m_OpaqueModels, *m_pOpaqueShaderProgram);
		m_pOpaqueFrameBuffer->unbind();

		//draw transparent objects


		//color blending
		CRenderer::getInstance()->clear();

		m_pOpaqueColorTexture->bindV(0);
		m_pColorBlendingProgram->bind();
		m_pColorBlendingProgram->updateUniformTexture("uOpaqueColorTex", m_pOpaqueColorTexture);

		CRenderer::getInstance()->drawScreenQuad(*m_pColorBlendingProgram);
	}

private:
	std::unique_ptr<CShaderProgram> m_pOpaqueShaderProgram;
	std::unique_ptr<CShaderProgram> m_pColorBlendingProgram;

	std::unique_ptr<CFrameBuffer>	m_pOpaqueFrameBuffer;

	std::shared_ptr<CTexture2D>		m_pOpaqueColorTexture;

	std::vector<std::shared_ptr<CModel>> m_OpaqueModels;
};

int main()
{
	CMyApplication App;
	if (!App.init(SWindowInfo(WIN_WIDTH, WIN_HEIGHT, "Order Independent Transparency"))) return -1;
	App.run();

	return 0;
}