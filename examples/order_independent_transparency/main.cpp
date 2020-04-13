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
		//draw skybox
		m_pOpaqueFrameBuffer->bind();
		CRenderer::getInstance()->clear();

		CRenderer::getInstance()->drawSkybox(*m_pSkybox, 0);

		//draw opaque objects
		//CRenderer::getInstance()->draw(m_OpaqueModels, *m_pOpaqueShaderProgram);

		//draw transparent objects
		m_pListHeadImage->clear();

		m_pGenLinkedListProgram->bind();
		m_pListAtomicCounter->reset();
		m_pGenLinkedListProgram->updateUniform1i("uMaxListNode", MAX_LIST_NODE);
		CRenderer::getInstance()->draw(m_TransparentModels, *m_pGenLinkedListProgram);
		m_pOpaqueFrameBuffer->unbind();

		//color blending
		CRenderer::getInstance()->clear();

		m_pOpaqueColorTexture->bindV(0);
		m_pColorBlendingProgram->bind();
		m_pColorBlendingProgram->updateUniformTexture("uOpaqueColorTex", m_pOpaqueColorTexture);

		CRenderer::getInstance()->drawScreenQuad(*m_pColorBlendingProgram);
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
	}

	void __initModels()
	{
		m_OpaqueModels.push_back(std::make_unique<CModel>("models/nanosuit/nanosuit.obj"));
		m_OpaqueModels[0]->setPosition(glm::vec3(0.0f, -1.5f, 0.0f));
		m_OpaqueModels[0]->setScale(glm::vec3(0.2f, 0.2f, 0.2f));

		m_TransparentModels.push_back(std::make_unique<CModel>("models/nanosuit/nanosuit.obj"));
		m_TransparentModels[0]->setPosition(glm::vec3(0.0f, -1.5f, 0.0f));
		m_TransparentModels[0]->setScale(glm::vec3(0.2f, 0.2f, 0.2f));
	}

	void __initTexturesAndBuffers()
	{
		m_pOpaqueColorTexture = std::make_shared<CTexture2D>();
		m_pOpaqueColorTexture->createEmpty(WIN_WIDTH, WIN_HEIGHT, GL_RGB8, GL_RGB);

		m_pOpaqueFrameBuffer = std::make_unique<CFrameBuffer>(WIN_WIDTH, WIN_HEIGHT);
		m_pOpaqueFrameBuffer->set(EAttachment::COLOR0, m_pOpaqueColorTexture);

		std::vector<std::string> Faces = {
			"textures/skybox/right.jpg", "textures/skybox/left.jpg", "textures/skybox/top.jpg", "textures/skybox/bottom.jpg", "textures/skybox/front.jpg", "textures/skybox/back.jpg"
		};
		m_pSkybox = std::make_unique<CSkybox>(Faces);

		m_pListHeadImage = std::make_unique<CImage2D>();
		m_pListHeadImage->createEmpty(WIN_WIDTH, WIN_HEIGHT, GL_R32UI, 0);

		m_pListAtomicCounter = std::make_unique<CAtomicCounterBuffer>(1);

		m_pListNodeBuffer = std::make_unique<CShaderStorageBuffer>(nullptr, MAX_LIST_NODE * sizeof(SListNode), 2);
	}

	std::unique_ptr<CShaderProgram> m_pOpaqueShaderProgram;
	std::unique_ptr<CShaderProgram> m_pGenLinkedListProgram;
	std::unique_ptr<CShaderProgram> m_pSortLinkedListProgram;
	std::unique_ptr<CShaderProgram> m_pColorBlendingProgram;
	std::unique_ptr<CFrameBuffer>	m_pOpaqueFrameBuffer;
	std::unique_ptr<CSkybox>		m_pSkybox;
	std::unique_ptr<CShaderStorageBuffer>	m_pListNodeBuffer;
	std::unique_ptr<CImage2D>				m_pListHeadImage;
	std::unique_ptr<CAtomicCounterBuffer>	m_pListAtomicCounter;

	std::shared_ptr<CTexture2D>		m_pOpaqueColorTexture;

	std::vector<std::shared_ptr<CModel>> m_OpaqueModels;
	std::vector<std::shared_ptr<CModel>> m_TransparentModels;
};

int main()
{
	CMyApplication App;
	if (!App.init(SWindowInfo(WIN_WIDTH, WIN_HEIGHT, "Order Independent Transparency"))) return -1;
	App.run();

	return 0;
}