#include <memory>
#include <vector>
#include "ApplicationBase.h"
#include "ShaderProgram.h"
#include "Model.h"
#include "Texture.h"
#include "FrameBuffer.h"
#include "Skybox.h"
#include "FileLocator.h"
#include "ShaderStorageBuffer.h"
#include "AtomicCounterBuffer.h"
#include "InputManager.h"

using namespace glt;

const int WIN_WIDTH = 1024;
const int WIN_HEIGHT = 576;
const int MAX_LIST_NODE = WIN_WIDTH * WIN_HEIGHT * 6;

struct SListNode
{
	unsigned packedColor;
	unsigned depthAndCoverage;
	unsigned next;
};

class CMyApplication : public CApplicationBase
{
protected:
	bool _initV() override
	{
		setDisplayStatusHint();

		CFileLocator::getInstance()->addFileSearchPath("../../resource");

		__initShaders();
		__initModels();
		__initTexturesAndBuffers();

		CRenderer::getInstance()->fetchCamera()->setPosition(glm::dvec3(0, 0, 5));

		return true;
	}

	void _renderV() override
	{
		__drawOpaqueObjects();
		__drawTransparentObjects();
		__mergeColor();
	}

	void _updateV() override
	{
		auto KeyStatus = CInputManager::getInstance()->getKeyStatus();
		if (KeyStatus[GLFW_KEY_0]) m_BlendingStrategy = 0;
		else if (KeyStatus[GLFW_KEY_1]) m_BlendingStrategy = 1;
		else if (KeyStatus[GLFW_KEY_2]) m_BlendingStrategy = 2;
	}

private:
	void __initShaders()
	{
		m_pOpaqueShaderProgram = std::make_unique<CShaderProgram>();
		m_pOpaqueShaderProgram->addShader("shaders/opaque_shading_vs.glsl", EShaderType::VERTEX_SHADER);
		m_pOpaqueShaderProgram->addShader("shaders/opaque_shading_fs.glsl", EShaderType::FRAGMENT_SHADER);

		m_pGenLinkedListProgram = std::make_unique<CShaderProgram>();
		m_pGenLinkedListProgram->addShader("shaders/generate_linked_list_vs.glsl", EShaderType::VERTEX_SHADER);
		m_pGenLinkedListProgram->addShader("shaders/generate_linked_list_fs.glsl", EShaderType::FRAGMENT_SHADER);

		m_pColorBlendingProgram = std::make_unique<CShaderProgram>();
		m_pColorBlendingProgram->addShader("shaders/draw_screen_quad_vs.glsl", EShaderType::VERTEX_SHADER);
		m_pColorBlendingProgram->addShader("shaders/color_blending_fs.glsl", EShaderType::FRAGMENT_SHADER);

		m_pMergeColorProgram = std::make_unique<CShaderProgram>();
		m_pMergeColorProgram->addShader("shaders/draw_screen_quad_vs.glsl", EShaderType::VERTEX_SHADER);
		m_pMergeColorProgram->addShader("shaders/merge_color_fs.glsl", EShaderType::FRAGMENT_SHADER);
	}

	void __initModels()
	{
		m_OpaqueModels.push_back(std::make_unique<CModel>("models/nanosuit/nanosuit.obj"));
		m_OpaqueModels[0]->setPosition(glm::vec3(0.0f, -1.5f, -1.0f));
		m_OpaqueModels[0]->setScale(glm::vec3(0.2f, 0.2f, 0.2f));

		m_TransparentModels.push_back(std::make_unique<CModel>("models/nanosuit/nanosuit.obj"));
		m_TransparentModels[0]->setPosition(glm::vec3(0.0f, -1.5f, 0.0f));
		m_TransparentModels[0]->setScale(glm::vec3(0.2f, 0.2f, 0.2f));
	}

	void __initTexturesAndBuffers()
	{
		m_pOpaqueColorTexture = std::make_shared<CTexture2D>();
		m_pOpaqueColorTexture->createEmpty(WIN_WIDTH, WIN_HEIGHT, GL_RGB8, GL_RGB, GL_FLOAT, GL_CLAMP_TO_BORDER, GL_NEAREST);

		m_pOpaqueDepthTexture = std::make_shared<CTexture2D>();
		m_pOpaqueDepthTexture->createEmpty(WIN_WIDTH, WIN_HEIGHT, GL_R16F, GL_RED, GL_FLOAT, GL_CLAMP_TO_BORDER, GL_NEAREST);

		m_pOpaqueFrameBuffer = std::make_unique<CFrameBuffer>(WIN_WIDTH, WIN_HEIGHT);
		m_pOpaqueFrameBuffer->set(EAttachment::COLOR0, m_pOpaqueColorTexture);
		m_pOpaqueFrameBuffer->set(EAttachment::COLOR1, m_pOpaqueDepthTexture);

		m_pTransparentColorTexture = std::make_shared<CTexture2D>();
		m_pTransparentColorTexture->createEmpty(WIN_WIDTH, WIN_HEIGHT, GL_RGBA8, GL_RGBA, GL_FLOAT, GL_CLAMP_TO_BORDER, GL_NEAREST);

		m_pTransparencyFrameBuffer = std::make_unique<CFrameBuffer>(WIN_WIDTH, WIN_HEIGHT);
		m_pTransparencyFrameBuffer->set(EAttachment::COLOR0, m_pTransparentColorTexture);

		std::vector<std::string> Faces = {
			"textures/skybox/right.jpg", "textures/skybox/left.jpg", "textures/skybox/top.jpg", "textures/skybox/bottom.jpg", "textures/skybox/front.jpg", "textures/skybox/back.jpg"
		};
		m_pSkybox = std::make_unique<CSkybox>(Faces);

		m_pListHeadImage = std::make_unique<CImage2D>();
		m_pListHeadImage->createEmpty(WIN_WIDTH, WIN_HEIGHT, GL_R32UI, 0);

		m_pListAtomicCounter = std::make_unique<CAtomicCounterBuffer>(1);

		m_pListNodeBuffer = std::make_unique<CShaderStorageBuffer>(nullptr, MAX_LIST_NODE * sizeof(SListNode), 2);
	}

	void __drawOpaqueObjects()
	{
		//draw skybox
		m_pOpaqueFrameBuffer->bind();
		CRenderer::getInstance()->clear();
		CRenderer::getInstance()->enableCullFace(true);
		CRenderer::getInstance()->drawSkybox(*m_pSkybox, 0);

		//draw opaque objects
		CRenderer::getInstance()->draw(m_OpaqueModels, *m_pOpaqueShaderProgram);
		m_pOpaqueFrameBuffer->unbind();
	}

	void __drawTransparentObjects()
	{
		m_pTransparencyFrameBuffer->bind();
		CRenderer::getInstance()->clear();
		CRenderer::getInstance()->enableCullFace(false);
		CRenderer::getInstance()->setDepthMask(false);
		m_pListHeadImage->clear();

		m_pGenLinkedListProgram->bind();
		m_pListAtomicCounter->reset();

		m_pGenLinkedListProgram->updateUniform1i("uMaxListNode", MAX_LIST_NODE);
		m_pOpaqueDepthTexture->bindV(2);
		m_pGenLinkedListProgram->updateUniformTexture("uOpaqueDepthTex", m_pOpaqueDepthTexture);
		CRenderer::getInstance()->draw(m_TransparentModels, *m_pGenLinkedListProgram);
		CRenderer::getInstance()->setDepthMask(true);

		m_pColorBlendingProgram->bind();
		m_pColorBlendingProgram->updateUniform1i("uBlendingStrategy", m_BlendingStrategy);
		CRenderer::getInstance()->drawScreenQuad(*m_pColorBlendingProgram);

		m_pTransparencyFrameBuffer->unbind();
	}

	void __mergeColor()
	{
		CRenderer::getInstance()->clear();

		m_pOpaqueColorTexture->bindV(0);
		m_pTransparentColorTexture->bindV(1);

		m_pMergeColorProgram->bind();
		m_pMergeColorProgram->updateUniformTexture("uOpaqueColorTex", m_pOpaqueColorTexture);
		m_pMergeColorProgram->updateUniformTexture("uTransparentColorTex", m_pTransparentColorTexture);

		CRenderer::getInstance()->drawScreenQuad(*m_pMergeColorProgram);
	}

	std::unique_ptr<CShaderProgram> m_pOpaqueShaderProgram;
	std::unique_ptr<CShaderProgram> m_pGenLinkedListProgram;
	std::unique_ptr<CShaderProgram> m_pColorBlendingProgram;
	std::unique_ptr<CShaderProgram> m_pMergeColorProgram;
	std::unique_ptr<CFrameBuffer>	m_pOpaqueFrameBuffer;
	std::unique_ptr<CFrameBuffer>	m_pTransparencyFrameBuffer;
	std::unique_ptr<CSkybox>		m_pSkybox;
	std::unique_ptr<CShaderStorageBuffer>	m_pListNodeBuffer;
	std::unique_ptr<CImage2D>				m_pListHeadImage;
	std::unique_ptr<CAtomicCounterBuffer>	m_pListAtomicCounter;

	std::shared_ptr<CTexture2D>		m_pOpaqueColorTexture;
	std::shared_ptr<CTexture2D>		m_pOpaqueDepthTexture;
	std::shared_ptr<CTexture2D>		m_pTransparentColorTexture;

	std::vector<std::shared_ptr<CModel>> m_OpaqueModels;
	std::vector<std::shared_ptr<CModel>> m_TransparentModels;

	int m_BlendingStrategy = 0;
};

int main()
{
	CMyApplication App;
	if (!App.init(SWindowInfo(WIN_WIDTH, WIN_HEIGHT, "Order Independent Transparency"))) return -1;
	App.run();

	return 0;
}