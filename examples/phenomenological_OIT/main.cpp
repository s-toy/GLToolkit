#include <memory>
#include <vector>
#include <map>
#include <glm/glm.hpp>
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
	unsigned transmittance;
	unsigned depth;
	unsigned next;
};

struct SMaterial
{
	glm::vec3 diffuse;
	glm::vec3 transmittance;
	float coverage;
};

class CMyApplication : public CApplicationBase
{
protected:
	bool _initV() override
	{
		setDisplayStatusHint();

		CFileLocator::getInstance()->addFileSearchPath("../../resource");

		__initShaders();
		__initScenes();
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

		m_pTransparencyShaderProgram = std::make_unique<CShaderProgram>();
		m_pTransparencyShaderProgram->addShader("shaders/transparency_shading_vs.glsl", EShaderType::VERTEX_SHADER);
		m_pTransparencyShaderProgram->addShader("shaders/transparency_shading_fs.glsl", EShaderType::FRAGMENT_SHADER);

		m_pMergeColorProgram = std::make_unique<CShaderProgram>();
		m_pMergeColorProgram->addShader("shaders/draw_screen_quad_vs.glsl", EShaderType::VERTEX_SHADER);
		m_pMergeColorProgram->addShader("shaders/merge_color_fs.glsl", EShaderType::FRAGMENT_SHADER);
	}

	void __initScenes()
	{
		std::vector<std::string> Faces = {
			"textures/skybox/right.jpg",
			"textures/skybox/left.jpg",
			"textures/skybox/top.jpg",
			"textures/skybox/bottom.jpg",
			"textures/skybox/front.jpg",
			"textures/skybox/back.jpg"
		};
		m_pSkybox = std::make_unique<CSkybox>(Faces);

		m_TransparentModels.push_back(std::make_unique<CModel>("models/nanosuit/nanosuit.obj"));
		m_TransparentModels.back()->setPosition(glm::vec3(2.0f, -1.5f, -1.0f));
		m_TransparentModels.back()->setScale(glm::vec3(0.2f, 0.2f, 0.2f));
		m_Model2MaterialMap[m_TransparentModels.back()] = SMaterial{ glm::vec3(0.6), glm::vec3(0.0), 0.2 };

		m_TransparentModels.push_back(std::make_unique<CModel>("models/nanosuit/nanosuit.obj"));
		m_TransparentModels.back()->setPosition(glm::vec3(2.0f, -1.5f, 0.0f));
		m_TransparentModels.back()->setScale(glm::vec3(0.2f, 0.2f, 0.2f));
		m_Model2MaterialMap[m_TransparentModels.back()] = SMaterial{ glm::vec3(0.0, 0.0, 0.6), glm::vec3(0.0), 0.8 };

		m_TransparentModels.push_back(std::make_unique<CModel>("models/nanosuit/nanosuit.obj"));
		m_TransparentModels.back()->setPosition(glm::vec3(0.0f, -1.5f, -1.0f));
		m_TransparentModels.back()->setScale(glm::vec3(0.2f, 0.2f, 0.2f));
		m_Model2MaterialMap[m_TransparentModels.back()] = SMaterial{ glm::vec3(0.6), glm::vec3(0.0), 0.5 };

		m_TransparentModels.push_back(std::make_unique<CModel>("models/nanosuit/nanosuit.obj"));
		m_TransparentModels.back()->setPosition(glm::vec3(0.0f, -1.5f, 0.0f));
		m_TransparentModels.back()->setScale(glm::vec3(0.2f, 0.2f, 0.2f));
		m_Model2MaterialMap[m_TransparentModels.back()] = SMaterial{ glm::vec3(0.0, 0.0, 0.6), glm::vec3(0.0), 0.5 };

		m_TransparentModels.push_back(std::make_unique<CModel>("models/nanosuit/nanosuit.obj"));
		m_TransparentModels.back()->setPosition(glm::vec3(-2.0f, -1.5f, -1.0f));
		m_TransparentModels.back()->setScale(glm::vec3(0.2f, 0.2f, 0.2f));
		m_Model2MaterialMap[m_TransparentModels.back()] = SMaterial{ glm::vec3(0.6), glm::vec3(0.0), 0.9 };

		m_TransparentModels.push_back(std::make_unique<CModel>("models/nanosuit/nanosuit.obj"));
		m_TransparentModels.back()->setPosition(glm::vec3(-2.0f, -1.5f, 0.0f));
		m_TransparentModels.back()->setScale(glm::vec3(0.2f, 0.2f, 0.2f));
		m_Model2MaterialMap[m_TransparentModels.back()] = SMaterial{ glm::vec3(0.0, 0.0, 0.6), glm::vec3(0.0), 0.1 };
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

		m_pAccumulatedReflectionColorTex = std::make_shared<CTexture2D>();
		m_pAccumulatedReflectionColorTex->createEmpty(WIN_WIDTH, WIN_HEIGHT, GL_RGBA16F, GL_RGBA, GL_FLOAT, GL_CLAMP_TO_BORDER, GL_NEAREST);

		m_pAccumulatedTransmissionTex = std::make_shared<CTexture2D>();
		m_pAccumulatedTransmissionTex->createEmpty(WIN_WIDTH, WIN_HEIGHT, GL_RGB8, GL_RGB, GL_FLOAT, GL_CLAMP_TO_BORDER, GL_NEAREST);

		m_pTransparencyFrameBuffer = std::make_unique<CFrameBuffer>(WIN_WIDTH, WIN_HEIGHT);
		m_pTransparencyFrameBuffer->set(EAttachment::COLOR0, m_pAccumulatedReflectionColorTex);
		m_pTransparencyFrameBuffer->set(EAttachment::COLOR1, m_pAccumulatedTransmissionTex);
	}

	void __drawOpaqueObjects()
	{
		//draw skybox
		m_pOpaqueFrameBuffer->bind();
		CRenderer::getInstance()->setClearColor(0, 0, 0, 0);
		CRenderer::getInstance()->clear();
		CRenderer::getInstance()->enableCullFace(true);
		if (m_pSkybox) CRenderer::getInstance()->drawSkybox(*m_pSkybox, 0);

		//draw opaque objects
		CRenderer::getInstance()->draw(m_OpaqueModels, *m_pOpaqueShaderProgram);
		m_pOpaqueFrameBuffer->unbind();
	}

	void __drawTransparentObjects()
	{
		m_pTransparencyFrameBuffer->bind();
		const float Zero[] = { 0, 0, 0, 0 };
		const float One[] = { 1, 1, 1 };

		CRenderer::getInstance()->setClearColor(0, 0, 0, 0);
		CRenderer::getInstance()->clear();
		CRenderer::getInstance()->clearBuffer(1, One);
		CRenderer::getInstance()->enableCullFace(false);
		CRenderer::getInstance()->setDepthMask(false);
		CRenderer::getInstance()->enableBlend(true);
		CRenderer::getInstance()->setBlendFunc(GL_ONE, GL_ONE, 0);
		CRenderer::getInstance()->setBlendFunc(GL_ZERO, GL_SRC_COLOR, 1);

		m_pTransparencyShaderProgram->bind();
		m_pOpaqueDepthTexture->bindV(2);
		m_pTransparencyShaderProgram->updateUniformTexture("uOpaqueDepthTex", m_pOpaqueDepthTexture.get());

		for (auto Model : m_TransparentModels)
		{
			auto Material = m_Model2MaterialMap[Model];
			m_pTransparencyShaderProgram->bind();
			m_pTransparencyShaderProgram->updateUniform3f("uDiffuseColor", Material.diffuse);
			m_pTransparencyShaderProgram->updateUniform3f("uTransmittance", Material.transmittance);
			m_pTransparencyShaderProgram->updateUniform1f("uCoverage", Material.coverage);
			CRenderer::getInstance()->draw(*Model, *m_pTransparencyShaderProgram);
		}

		CRenderer::getInstance()->setDepthMask(true);
		CRenderer::getInstance()->enableBlend(false);

		m_pTransparencyFrameBuffer->unbind();
	}

	void __mergeColor()
	{
		CRenderer::getInstance()->setClearColor(0, 0, 0, 0);
		CRenderer::getInstance()->clear();

		m_pOpaqueColorTexture->bindV(0);
		m_pAccumulatedReflectionColorTex->bindV(1);
		m_pAccumulatedTransmissionTex->bindV(2);

		m_pMergeColorProgram->bind();
		m_pMergeColorProgram->updateUniformTexture("uOpaqueColorTex", m_pOpaqueColorTexture.get());
		m_pMergeColorProgram->updateUniformTexture("uAccumulatedReflectionTex", m_pAccumulatedReflectionColorTex.get());
		m_pMergeColorProgram->updateUniformTexture("uAccumulatedTransmissionTex", m_pAccumulatedTransmissionTex.get());

		CRenderer::getInstance()->drawScreenQuad(*m_pMergeColorProgram);
	}

	std::unique_ptr<CShaderProgram> m_pOpaqueShaderProgram;
	std::unique_ptr<CShaderProgram> m_pTransparencyShaderProgram;
	std::unique_ptr<CShaderProgram> m_pMergeColorProgram;
	std::unique_ptr<CFrameBuffer>	m_pOpaqueFrameBuffer;
	std::unique_ptr<CFrameBuffer>	m_pTransparencyFrameBuffer;
	std::unique_ptr<CSkybox>		m_pSkybox;

	std::shared_ptr<CTexture2D>		m_pOpaqueColorTexture;
	std::shared_ptr<CTexture2D>		m_pOpaqueDepthTexture;
	std::shared_ptr<CTexture2D>		m_pAccumulatedReflectionColorTex;
	std::shared_ptr<CTexture2D>		m_pAccumulatedTransmissionTex;

	std::vector<std::shared_ptr<CModel>> m_OpaqueModels;
	std::vector<std::shared_ptr<CModel>> m_TransparentModels;

	int m_BlendingStrategy = 0;

	std::map<std::shared_ptr<CModel>, SMaterial> m_Model2MaterialMap;
};

int main()
{
	CMyApplication App;
	if (!App.init(SWindowInfo(WIN_WIDTH, WIN_HEIGHT, "Order Independent Transparency"))) return -1;
	App.run();

	return 0;
}