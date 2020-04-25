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
#include "InputManager.h"
#include "CpuTimer.h"

using namespace glt;

const int WIN_WIDTH = 1024;
const int WIN_HEIGHT = 576;

struct SMaterial
{
	glm::vec3 diffuse;
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
	}

private:
	void __initShaders()
	{
		m_pOpaqueShaderProgram = std::make_unique<CShaderProgram>();
		m_pOpaqueShaderProgram->addShader("shaders/draw_opaque_objects.vert", EShaderType::VERTEX_SHADER);
		m_pOpaqueShaderProgram->addShader("shaders/draw_opaque_objects.frag", EShaderType::FRAGMENT_SHADER);

		m_pTransparencyShaderProgram1 = std::make_unique<CShaderProgram>();
		m_pTransparencyShaderProgram1->addShader("shaders/transparency_pass_1.vert", EShaderType::VERTEX_SHADER);
		m_pTransparencyShaderProgram1->addShader("shaders/transparency_pass_1.frag", EShaderType::FRAGMENT_SHADER);

		m_pTransparencyShaderProgram2 = std::make_unique<CShaderProgram>();
		m_pTransparencyShaderProgram2->addShader("shaders/transparency_pass_2.vert", EShaderType::VERTEX_SHADER);
		m_pTransparencyShaderProgram2->addShader("shaders/transparency_pass_2.frag", EShaderType::FRAGMENT_SHADER);

		m_pMergeColorProgram = std::make_unique<CShaderProgram>();
		m_pMergeColorProgram->addShader("shaders/merge_color.vert", EShaderType::VERTEX_SHADER);
		m_pMergeColorProgram->addShader("shaders/merge_color.frag", EShaderType::FRAGMENT_SHADER);
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

		m_OpaqueModels.push_back(std::make_shared<CModel>("models/nanosuit/nanosuit.obj"));
		m_OpaqueModels.back()->setPosition(glm::vec3(1.0f, -1.5f, -0.5f));
		m_OpaqueModels.back()->setScale(glm::vec3(0.2f, 0.2f, 0.2f));

		m_OpaqueModels.push_back(std::make_shared<CModel>("models/nanosuit/nanosuit.obj"));
		m_OpaqueModels.back()->setPosition(glm::vec3(-1.0f, -1.5f, -0.5f));
		m_OpaqueModels.back()->setScale(glm::vec3(0.2f, 0.2f, 0.2f));

		m_TransparentModels.push_back(std::make_shared<CModel>("models/nanosuit/nanosuit.obj"));
		m_TransparentModels.back()->setPosition(glm::vec3(2.0f, -1.5f, -1.0f));
		m_TransparentModels.back()->setScale(glm::vec3(0.2f, 0.2f, 0.2f));
		m_Model2MaterialMap[m_TransparentModels.back()] = SMaterial{ glm::vec3(0.6), 0.2 };

		m_TransparentModels.push_back(std::make_shared<CModel>("models/nanosuit/nanosuit.obj"));
		m_TransparentModels.back()->setPosition(glm::vec3(2.0f, -1.5f, 0.0f));
		m_TransparentModels.back()->setScale(glm::vec3(0.2f, 0.2f, 0.2f));
		m_Model2MaterialMap[m_TransparentModels.back()] = SMaterial{ glm::vec3(0.0, 0.0, 0.6), 0.8 };

		m_TransparentModels.push_back(std::make_shared<CModel>("models/nanosuit/nanosuit.obj"));
		m_TransparentModels.back()->setPosition(glm::vec3(0.0f, -1.5f, -1.0f));
		m_TransparentModels.back()->setScale(glm::vec3(0.2f, 0.2f, 0.2f));
		m_Model2MaterialMap[m_TransparentModels.back()] = SMaterial{ glm::vec3(0.6), 0.5 };

		m_TransparentModels.push_back(std::make_shared<CModel>("models/nanosuit/nanosuit.obj"));
		m_TransparentModels.back()->setPosition(glm::vec3(0.0f, -1.5f, 0.0f));
		m_TransparentModels.back()->setScale(glm::vec3(0.2f, 0.2f, 0.2f));
		m_Model2MaterialMap[m_TransparentModels.back()] = SMaterial{ glm::vec3(0.0, 0.0, 0.6), 0.5 };

		m_TransparentModels.push_back(std::make_shared<CModel>("models/nanosuit/nanosuit.obj"));
		m_TransparentModels.back()->setPosition(glm::vec3(-2.0f, -1.5f, -1.0f));
		m_TransparentModels.back()->setScale(glm::vec3(0.2f, 0.2f, 0.2f));
		m_Model2MaterialMap[m_TransparentModels.back()] = SMaterial{ glm::vec3(0.6), 0.9 };

		m_TransparentModels.push_back(std::make_shared<CModel>("models/nanosuit/nanosuit.obj"));
		m_TransparentModels.back()->setPosition(glm::vec3(-2.0f, -1.5f, 0.0f));
		m_TransparentModels.back()->setScale(glm::vec3(0.2f, 0.2f, 0.2f));
		m_Model2MaterialMap[m_TransparentModels.back()] = SMaterial{ glm::vec3(0.0, 0.0, 0.6), 0.1 };
	}

	void __initTexturesAndBuffers()
	{
		m_pOpaqueColorTex = std::make_shared<CTexture2D>();
		m_pOpaqueColorTex->createEmpty(WIN_WIDTH, WIN_HEIGHT, GL_RGB8, GL_CLAMP_TO_BORDER, GL_NEAREST);

		m_pOpaqueDepthTex = std::make_shared<CTexture2D>();
		m_pOpaqueDepthTex->createEmpty(WIN_WIDTH, WIN_HEIGHT, GL_R16F, GL_CLAMP_TO_BORDER, GL_NEAREST);

		m_pOpaqueFrameBuffer = std::make_unique<CFrameBuffer>(WIN_WIDTH, WIN_HEIGHT);
		m_pOpaqueFrameBuffer->set(EAttachment::COLOR0, m_pOpaqueColorTex);
		m_pOpaqueFrameBuffer->set(EAttachment::COLOR1, m_pOpaqueDepthTex);

		m_pMomentB0Tex = std::make_shared<CTexture2D>();
		m_pMomentB0Tex->createEmpty(WIN_WIDTH, WIN_HEIGHT, GL_R16F, GL_CLAMP_TO_BORDER, GL_NEAREST);

		m_pTransparencyFrameBuffer1 = std::make_unique<CFrameBuffer>(WIN_WIDTH, WIN_HEIGHT);
		m_pTransparencyFrameBuffer1->set(EAttachment::COLOR0, m_pMomentB0Tex);

		m_pTransparencyColorTex = std::make_shared<CTexture2D>();
		m_pTransparencyColorTex->createEmpty(WIN_WIDTH, WIN_HEIGHT, GL_RGBA16F, GL_CLAMP_TO_BORDER, GL_NEAREST);

		m_pTransparencyFrameBuffer2 = std::make_unique<CFrameBuffer>(WIN_WIDTH, WIN_HEIGHT);
		m_pTransparencyFrameBuffer2->set(EAttachment::COLOR0, m_pTransparencyColorTex);
	}

	void __drawOpaqueObjects()
	{
		//draw skybox
		m_pOpaqueFrameBuffer->bind();
		CRenderer::getInstance()->clear();
		CRenderer::getInstance()->enableCullFace(true);
		if (m_pSkybox) CRenderer::getInstance()->drawSkybox(*m_pSkybox, 0);

		//draw opaque objects
		CRenderer::getInstance()->draw(m_OpaqueModels, *m_pOpaqueShaderProgram);
		m_pOpaqueFrameBuffer->unbind();
	}

	void __drawTransparentObjects()
	{
		//pass1: generate moments
		m_pTransparencyFrameBuffer1->bind();

		CRenderer::getInstance()->clear();
		CRenderer::getInstance()->enableCullFace(false);
		CRenderer::getInstance()->setDepthMask(false);
		CRenderer::getInstance()->enableBlend(true);
		CRenderer::getInstance()->setBlendFunc(GL_ONE, GL_ONE);

		m_pTransparencyShaderProgram1->bind();
		m_pOpaqueDepthTex->bindV(2);
		m_pTransparencyShaderProgram1->updateUniformTexture("uOpaqueDepthTex", m_pOpaqueDepthTex.get());

		for (auto Model : m_TransparentModels)
		{
			auto Material = m_Model2MaterialMap[Model];
			m_pTransparencyShaderProgram1->bind();
			m_pTransparencyShaderProgram1->updateUniform1f("uCoverage", Material.coverage);
			CRenderer::getInstance()->draw(*Model, *m_pTransparencyShaderProgram1);
		}

		CRenderer::getInstance()->setDepthMask(true);
		CRenderer::getInstance()->enableBlend(false);

		m_pTransparencyFrameBuffer1->unbind();

		//pass2: reconstruct transmittance
		m_pTransparencyFrameBuffer2->bind();

		CRenderer::getInstance()->clear();
		CRenderer::getInstance()->enableCullFace(false);
		CRenderer::getInstance()->setDepthMask(false);
		CRenderer::getInstance()->enableBlend(true);
		CRenderer::getInstance()->setBlendFunc(GL_ONE, GL_ONE);

		m_pTransparencyShaderProgram2->bind();
		m_pOpaqueDepthTex->bindV(2);
		m_pTransparencyShaderProgram2->updateUniformTexture("uOpaqueDepthTex", m_pOpaqueDepthTex.get());

		for (auto Model : m_TransparentModels)
		{
			auto Material = m_Model2MaterialMap[Model];
			m_pTransparencyShaderProgram2->bind();
			m_pTransparencyShaderProgram2->updateUniform3f("uDiffuseColor", Material.diffuse);
			m_pTransparencyShaderProgram2->updateUniform1f("uCoverage", Material.coverage);
			CRenderer::getInstance()->draw(*Model, *m_pTransparencyShaderProgram2);
		}

		CRenderer::getInstance()->setDepthMask(true);
		CRenderer::getInstance()->enableBlend(false);

		m_pTransparencyFrameBuffer2->unbind();
	}

	void __mergeColor()
	{
		CRenderer::getInstance()->clear();

		m_pOpaqueColorTex->bindV(0);
		m_pTransparencyColorTex->bindV(1);
		m_pMomentB0Tex->bindV(2);

		m_pMergeColorProgram->bind();
		m_pMergeColorProgram->updateUniformTexture("uOpaqueColorTex", m_pOpaqueColorTex.get());
		m_pMergeColorProgram->updateUniformTexture("uTranslucentColorTex", m_pTransparencyColorTex.get());
		m_pMergeColorProgram->updateUniformTexture("uMomentB0Tex", m_pMomentB0Tex.get());

		CRenderer::getInstance()->drawScreenQuad(*m_pMergeColorProgram);
	}

	std::unique_ptr<CShaderProgram> m_pOpaqueShaderProgram;
	std::unique_ptr<CShaderProgram> m_pTransparencyShaderProgram1;
	std::unique_ptr<CShaderProgram> m_pTransparencyShaderProgram2;
	std::unique_ptr<CShaderProgram> m_pMergeColorProgram;

	std::unique_ptr<CFrameBuffer>	m_pOpaqueFrameBuffer;
	std::unique_ptr<CFrameBuffer>	m_pTransparencyFrameBuffer1;
	std::unique_ptr<CFrameBuffer>	m_pTransparencyFrameBuffer2;

	std::shared_ptr<CTexture2D>		m_pOpaqueColorTex;
	std::shared_ptr<CTexture2D>		m_pOpaqueDepthTex;
	std::shared_ptr<CTexture2D>		m_pMomentB0Tex;
	std::shared_ptr<CTexture2D>		m_pTransparencyColorTex;

	std::unique_ptr<CSkybox>		m_pSkybox;
	std::vector<std::shared_ptr<CModel>> m_OpaqueModels;
	std::vector<std::shared_ptr<CModel>> m_TransparentModels;

	std::map<std::shared_ptr<CModel>, SMaterial> m_Model2MaterialMap;
};

int main()
{
	CMyApplication App;
	if (!App.init(SWindowInfo(WIN_WIDTH, WIN_HEIGHT, "Moment-based OIT"))) return -1;
	App.run();

	return 0;
}