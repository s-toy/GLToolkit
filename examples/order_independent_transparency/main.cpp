#include <memory>
#include <vector>
#include <map>
#include <glm/glm.hpp>
#include "ApplicationBase.h"
#include "ShaderProgram.h"
#include "ShaderStorageBuffer.h"
#include "AtomicCounterBuffer.h"
#include "Model.h"
#include "Texture.h"
#include "FrameBuffer.h"
#include "Skybox.h"
#include "FileLocator.h"
#include "InputManager.h"
#include "CpuTimer.h"

#define USING_ALL_METHODS

#ifdef USING_ALL_METHODS
#define USING_MOMENT_BASED_OIT
#define USING_WEIGHTED_BLENDED_OIT
#define USING_LINKED_LIST_OIT
#endif

enum class EOITMethod : unsigned char
{
	LINKED_LIST_OIT = 0,
	MOMENT_BASE_OIT,
	WEIGHTED_BLENDED_OIT
};

using namespace glt;

const int WIN_WIDTH = 1024;
const int WIN_HEIGHT = 576;

#ifdef USING_LINKED_LIST_OIT
const int MAX_LIST_NODE = WIN_WIDTH * WIN_HEIGHT * 6;

struct SListNode
{
	unsigned packedColor;
	unsigned transmittance;
	unsigned depth;
	unsigned next;
};
#endif

struct SMaterial
{
	SMaterial() {}

	SMaterial(glm::vec3 _diffuse, glm::vec3 _transmittance, float _coverage)
		: diffuse(_diffuse), transmittance(_transmittance), coverage(_coverage) {}

	SMaterial(glm::vec3 _diffuse, float _coverage)
		: diffuse(_diffuse), coverage(_coverage) {}

	glm::vec3 diffuse = glm::vec3(0.0);
	glm::vec3 transmittance = glm::vec3(0.0);
	float coverage = 0.0;
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

		CRenderer::getInstance()->fetchCamera()->setPosition(glm::dvec3(0, 0, 4));

		return true;
	}

	void _renderV() override
	{
		__drawOpaqueObjects();
		__drawTransparentObjects();
	}

	void _updateV() override
	{
		auto KeyStatus = CInputManager::getInstance()->getKeyStatus();

#ifdef USING_ALL_METHODS
		if (KeyStatus[GLFW_KEY_L]) m_OITMethod = EOITMethod::LINKED_LIST_OIT;
		else if (KeyStatus[GLFW_KEY_M]) m_OITMethod = EOITMethod::MOMENT_BASE_OIT;
		else if (KeyStatus[GLFW_KEY_B]) m_OITMethod = EOITMethod::WEIGHTED_BLENDED_OIT;
#endif

#ifdef USING_MOMENT_BASED_OIT
		if (KeyStatus[GLFW_KEY_4]) m_ReconstructionStrategy = 0;
		else if (KeyStatus[GLFW_KEY_6]) m_ReconstructionStrategy = 1;
		else if (KeyStatus[GLFW_KEY_8]) m_ReconstructionStrategy = 2;
#endif

#ifdef USING_WEIGHTED_BLENDED_OIT
		if (KeyStatus[GLFW_KEY_0]) m_WeightingStrategy = 0;
		else if (KeyStatus[GLFW_KEY_1]) m_WeightingStrategy = 1;
#endif
	}

private:
	void __initShaders()
	{
		m_pOpaqueShaderProgram = std::make_unique<CShaderProgram>();
		m_pOpaqueShaderProgram->addShader("shaders/draw_opaque_objects.vert", EShaderType::VERTEX_SHADER);
		m_pOpaqueShaderProgram->addShader("shaders/draw_opaque_objects.frag", EShaderType::FRAGMENT_SHADER);

#ifdef USING_MOMENT_BASED_OIT
		m_pGenerateMomentShaderProgram = std::make_unique<CShaderProgram>();
		m_pGenerateMomentShaderProgram->addShader("shaders/MBOIT_generate_moments.vert", EShaderType::VERTEX_SHADER);
		m_pGenerateMomentShaderProgram->addShader("shaders/MBOIT_generate_moments.frag", EShaderType::FRAGMENT_SHADER);

		m_pReconstructTransmittanceShaderProgram = std::make_unique<CShaderProgram>();
		m_pReconstructTransmittanceShaderProgram->addShader("shaders/MBOIT_reconstruct_transmittance.vert", EShaderType::VERTEX_SHADER);
		m_pReconstructTransmittanceShaderProgram->addShader("shaders/MBOIT_reconstruct_transmittance.frag", EShaderType::FRAGMENT_SHADER);

		m_pMBOITMergeColorShaderProgram = std::make_unique<CShaderProgram>();
		m_pMBOITMergeColorShaderProgram->addShader("shaders/draw_screen_coord.vert", EShaderType::VERTEX_SHADER);
		m_pMBOITMergeColorShaderProgram->addShader("shaders/MBOIT_merge_color.frag", EShaderType::FRAGMENT_SHADER);
#endif

#ifdef USING_WEIGHTED_BLENDED_OIT
		m_pWeightedBlendingShaderProgram = std::make_unique<CShaderProgram>();
		m_pWeightedBlendingShaderProgram->addShader("shaders/WBOIT_weighted_blending.vert", EShaderType::VERTEX_SHADER);
		m_pWeightedBlendingShaderProgram->addShader("shaders/WBOIT_weighted_blending.frag", EShaderType::FRAGMENT_SHADER);

		m_pWBOITMergeColorShaderProgram = std::make_unique<CShaderProgram>();
		m_pWBOITMergeColorShaderProgram->addShader("shaders/draw_screen_coord.vert", EShaderType::VERTEX_SHADER);
		m_pWBOITMergeColorShaderProgram->addShader("shaders/WBOIT_merge_color.frag", EShaderType::FRAGMENT_SHADER);
#endif

#ifdef USING_LINKED_LIST_OIT
		m_pGenLinkedListShaderProgram = std::make_unique<CShaderProgram>();
		m_pGenLinkedListShaderProgram->addShader("shaders/LLOIT_generate_linked_list.vert", EShaderType::VERTEX_SHADER);
		m_pGenLinkedListShaderProgram->addShader("shaders/LLOIT_generate_linked_list.frag", EShaderType::FRAGMENT_SHADER);

		m_pColorBlendingShaderProgram = std::make_unique<CShaderProgram>();
		m_pColorBlendingShaderProgram->addShader("shaders/draw_screen_coord.vert", EShaderType::VERTEX_SHADER);
		m_pColorBlendingShaderProgram->addShader("shaders/LLOIT_color_blending.frag", EShaderType::FRAGMENT_SHADER);

		m_pLLOITMergeColorShaderProgram = std::make_unique<CShaderProgram>();
		m_pLLOITMergeColorShaderProgram->addShader("shaders/draw_screen_coord.vert", EShaderType::VERTEX_SHADER);
		m_pLLOITMergeColorShaderProgram->addShader("shaders/LLOIT_merge_color.frag", EShaderType::FRAGMENT_SHADER);
#endif
	}

	void __initScenes()
	{
		__initScene02();
	}

	void __initScene01()
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

		//right
		m_TransparentModels.push_back(std::make_unique<CModel>("models/nanosuit/nanosuit.obj"));
		m_TransparentModels.back()->setPosition(glm::vec3(2.0f, -1.5f, -1.0f));
		m_TransparentModels.back()->setScale(glm::vec3(0.2f, 0.2f, 0.2f));
		m_Model2MaterialMap[m_TransparentModels.back()] = SMaterial{ glm::vec3(0.6), 0.2 };

		m_TransparentModels.push_back(std::make_unique<CModel>("models/nanosuit/nanosuit.obj"));
		m_TransparentModels.back()->setPosition(glm::vec3(2.0f, -1.5f, 0.0f));
		m_TransparentModels.back()->setScale(glm::vec3(0.2f, 0.2f, 0.2f));
		m_Model2MaterialMap[m_TransparentModels.back()] = SMaterial{ glm::vec3(0.0, 0.0, 0.6), 0.8 };

		//middle
		m_TransparentModels.push_back(std::make_unique<CModel>("models/nanosuit/nanosuit.obj"));
		m_TransparentModels.back()->setPosition(glm::vec3(0.0f, -1.5f, -1.0f));
		m_TransparentModels.back()->setScale(glm::vec3(0.2f, 0.2f, 0.2f));
		m_Model2MaterialMap[m_TransparentModels.back()] = SMaterial{ glm::vec3(0.6), 0.5 };

		m_TransparentModels.push_back(std::make_unique<CModel>("models/nanosuit/nanosuit.obj"));
		m_TransparentModels.back()->setPosition(glm::vec3(0.0f, -1.5f, 0.0f));
		m_TransparentModels.back()->setScale(glm::vec3(0.2f, 0.2f, 0.2f));
		m_Model2MaterialMap[m_TransparentModels.back()] = SMaterial{ glm::vec3(0.0, 0.0, 0.6), 0.5 };

		//left
		m_TransparentModels.push_back(std::make_unique<CModel>("models/nanosuit/nanosuit.obj"));
		m_TransparentModels.back()->setPosition(glm::vec3(-2.0f, -1.5f, -1.0f));
		m_TransparentModels.back()->setScale(glm::vec3(0.2f, 0.2f, 0.2f));
		m_Model2MaterialMap[m_TransparentModels.back()] = SMaterial{ glm::vec3(0.6), 0.9 };

		m_TransparentModels.push_back(std::make_unique<CModel>("models/nanosuit/nanosuit.obj"));
		m_TransparentModels.back()->setPosition(glm::vec3(-2.0f, -1.5f, 0.0f));
		m_TransparentModels.back()->setScale(glm::vec3(0.2f, 0.2f, 0.2f));
		m_Model2MaterialMap[m_TransparentModels.back()] = SMaterial{ glm::vec3(0.0, 0.0, 0.6), 0.1 };
	}

	void __initScene02()
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

		//middle
		m_TransparentModels.push_back(std::make_shared<CModel>("models/plane/plane.obj"));
		m_TransparentModels.back()->setPosition(glm::vec3(0.0f, 0.0f, -1.0f));
		m_TransparentModels.back()->setScale(glm::vec3(0.6f, 0.6f, 0.6f));
		m_Model2MaterialMap[m_TransparentModels.back()] = SMaterial(glm::vec3(0.5), 1.0);

		m_TransparentModels.push_back(std::make_shared<CModel>("models/plane/plane.obj"));
		m_TransparentModels.back()->setPosition(glm::vec3(0.0f, 0.0f, -0.5f));
		m_TransparentModels.back()->setScale(glm::vec3(0.6f, 0.6f, 0.6f));
		m_Model2MaterialMap[m_TransparentModels.back()] = SMaterial(glm::vec3(0.0, 0.5, 0.0), 1.0);

		m_TransparentModels.push_back(std::make_shared<CModel>("models/plane/plane.obj"));
		m_TransparentModels.back()->setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
		m_TransparentModels.back()->setScale(glm::vec3(0.6f, 0.6f, 0.6f));
		m_Model2MaterialMap[m_TransparentModels.back()] = SMaterial(glm::vec3(0.5, 0.0, 0.0), 1.0);

		//left
		m_TransparentModels.push_back(std::make_shared<CModel>("models/plane/plane.obj"));
		m_TransparentModels.back()->setPosition(glm::vec3(-2.0f, 0.0f, -1.0f));
		m_TransparentModels.back()->setScale(glm::vec3(0.6f, 0.6f, 0.6f));
		m_Model2MaterialMap[m_TransparentModels.back()] = SMaterial(glm::vec3(0.5), 0.8);

		m_TransparentModels.push_back(std::make_shared<CModel>("models/plane/plane.obj"));
		m_TransparentModels.back()->setPosition(glm::vec3(-2.0f, 0.0f, -0.5f));
		m_TransparentModels.back()->setScale(glm::vec3(0.6f, 0.6f, 0.6f));
		m_Model2MaterialMap[m_TransparentModels.back()] = SMaterial(glm::vec3(0.0, 0.5, 0.0), 0.5);

		m_TransparentModels.push_back(std::make_shared<CModel>("models/plane/plane.obj"));
		m_TransparentModels.back()->setPosition(glm::vec3(-2.0f, 0.0f, 0.0f));
		m_TransparentModels.back()->setScale(glm::vec3(0.6f, 0.6f, 0.6f));
		m_Model2MaterialMap[m_TransparentModels.back()] = SMaterial(glm::vec3(0.5, 0.0, 0.0), 0.2);

		//right
		m_TransparentModels.push_back(std::make_shared<CModel>("models/plane/plane.obj"));
		m_TransparentModels.back()->setPosition(glm::vec3(2.0f, 0.0f, -1.0f));
		m_TransparentModels.back()->setScale(glm::vec3(0.6f, 0.6f, 0.6f));
		m_Model2MaterialMap[m_TransparentModels.back()] = SMaterial(glm::vec3(0.5), 0.2);

		m_TransparentModels.push_back(std::make_shared<CModel>("models/plane/plane.obj"));
		m_TransparentModels.back()->setPosition(glm::vec3(2.0f, 0.0f, -0.5f));
		m_TransparentModels.back()->setScale(glm::vec3(0.6f, 0.6f, 0.6f));
		m_Model2MaterialMap[m_TransparentModels.back()] = SMaterial(glm::vec3(0.0, 0.5, 0.0), 0.5);

		m_TransparentModels.push_back(std::make_shared<CModel>("models/plane/plane.obj"));
		m_TransparentModels.back()->setPosition(glm::vec3(2.0f, 0.0f, 0.0f));
		m_TransparentModels.back()->setScale(glm::vec3(0.6f, 0.6f, 0.6f));
		m_Model2MaterialMap[m_TransparentModels.back()] = SMaterial(glm::vec3(0.5, 0.0, 0.0), 0.8);
	}

	void __initScene03()
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

		//middle
		m_TransparentModels.push_back(std::make_shared<CModel>("models/plane/plane.obj"));
		m_TransparentModels.back()->setPosition(glm::vec3(0.0f, 0.0f, -0.02f));
		m_TransparentModels.back()->setScale(glm::vec3(0.6f, 0.6f, 0.6f));
		m_Model2MaterialMap[m_TransparentModels.back()] = SMaterial(glm::vec3(0.5), 1.0);

		m_TransparentModels.push_back(std::make_shared<CModel>("models/plane/plane.obj"));
		m_TransparentModels.back()->setPosition(glm::vec3(0.0f, 0.0f, -0.01f));
		m_TransparentModels.back()->setScale(glm::vec3(0.6f, 0.6f, 0.6f));
		m_Model2MaterialMap[m_TransparentModels.back()] = SMaterial(glm::vec3(0.0, 0.5, 0.0), 1.0);

		m_TransparentModels.push_back(std::make_shared<CModel>("models/plane/plane.obj"));
		m_TransparentModels.back()->setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
		m_TransparentModels.back()->setScale(glm::vec3(0.6f, 0.6f, 0.6f));
		m_Model2MaterialMap[m_TransparentModels.back()] = SMaterial(glm::vec3(0.5, 0.0, 0.0), 1.0);

		//left
		m_TransparentModels.push_back(std::make_shared<CModel>("models/plane/plane.obj"));
		m_TransparentModels.back()->setPosition(glm::vec3(-2.0f, 0.0f, -0.02f));
		m_TransparentModels.back()->setScale(glm::vec3(0.6f, 0.6f, 0.6f));
		m_Model2MaterialMap[m_TransparentModels.back()] = SMaterial(glm::vec3(0.5), 0.8);

		m_TransparentModels.push_back(std::make_shared<CModel>("models/plane/plane.obj"));
		m_TransparentModels.back()->setPosition(glm::vec3(-2.0f, 0.0f, -0.01f));
		m_TransparentModels.back()->setScale(glm::vec3(0.6f, 0.6f, 0.6f));
		m_Model2MaterialMap[m_TransparentModels.back()] = SMaterial(glm::vec3(0.0, 0.5, 0.0), 0.5);

		m_TransparentModels.push_back(std::make_shared<CModel>("models/plane/plane.obj"));
		m_TransparentModels.back()->setPosition(glm::vec3(-2.0f, 0.0f, 0.0f));
		m_TransparentModels.back()->setScale(glm::vec3(0.6f, 0.6f, 0.6f));
		m_Model2MaterialMap[m_TransparentModels.back()] = SMaterial(glm::vec3(0.5, 0.0, 0.0), 0.2);

		//right
		m_TransparentModels.push_back(std::make_shared<CModel>("models/plane/plane.obj"));
		m_TransparentModels.back()->setPosition(glm::vec3(2.0f, 0.0f, -0.02f));
		m_TransparentModels.back()->setScale(glm::vec3(0.6f, 0.6f, 0.6f));
		m_Model2MaterialMap[m_TransparentModels.back()] = SMaterial(glm::vec3(0.5), 0.2);

		m_TransparentModels.push_back(std::make_shared<CModel>("models/plane/plane.obj"));
		m_TransparentModels.back()->setPosition(glm::vec3(2.0f, 0.0f, -0.01f));
		m_TransparentModels.back()->setScale(glm::vec3(0.6f, 0.6f, 0.6f));
		m_Model2MaterialMap[m_TransparentModels.back()] = SMaterial(glm::vec3(0.0, 0.5, 0.0), 0.5);

		m_TransparentModels.push_back(std::make_shared<CModel>("models/plane/plane.obj"));
		m_TransparentModels.back()->setPosition(glm::vec3(2.0f, 0.0f, 0.0f));
		m_TransparentModels.back()->setScale(glm::vec3(0.6f, 0.6f, 0.6f));
		m_Model2MaterialMap[m_TransparentModels.back()] = SMaterial(glm::vec3(0.5, 0.0, 0.0), 0.8);
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

		m_pTransparencyColorTex = std::make_shared<CTexture2D>();
		m_pTransparencyColorTex->createEmpty(WIN_WIDTH, WIN_HEIGHT, GL_RGBA16F, GL_CLAMP_TO_BORDER, GL_NEAREST);

#ifdef USING_MOMENT_BASED_OIT
		m_pMomentB0Tex = std::make_shared<CTexture2D>();
		m_pMomentB0Tex->createEmpty(WIN_WIDTH, WIN_HEIGHT, GL_R32F, GL_CLAMP_TO_BORDER, GL_NEAREST);

		m_pMomentsTex = std::make_shared<CTexture2D>();
		m_pMomentsTex->createEmpty(WIN_WIDTH, WIN_HEIGHT, GL_RGBA32F, GL_CLAMP_TO_BORDER, GL_NEAREST);

		m_pExtraMomentsTex = std::make_shared<CTexture2D>();
		m_pExtraMomentsTex->createEmpty(WIN_WIDTH, WIN_HEIGHT, GL_RGBA32F, GL_CLAMP_TO_BORDER, GL_NEAREST);

		m_pMBOITFrameBuffer1 = std::make_unique<CFrameBuffer>(WIN_WIDTH, WIN_HEIGHT);
		m_pMBOITFrameBuffer1->set(EAttachment::COLOR0, m_pMomentB0Tex);
		m_pMBOITFrameBuffer1->set(EAttachment::COLOR1, m_pMomentsTex);
		m_pMBOITFrameBuffer1->set(EAttachment::COLOR2, m_pExtraMomentsTex);

		m_pMBOITFrameBuffer2 = std::make_unique<CFrameBuffer>(WIN_WIDTH, WIN_HEIGHT);
		m_pMBOITFrameBuffer2->set(EAttachment::COLOR0, m_pTransparencyColorTex);
#endif

#ifdef USING_WEIGHTED_BLENDED_OIT
		m_pAccumulatedTransmittanceTex = std::make_shared<CTexture2D>();
		m_pAccumulatedTransmittanceTex->createEmpty(WIN_WIDTH, WIN_HEIGHT, GL_RGB16F, GL_CLAMP_TO_BORDER, GL_NEAREST);

		m_pWBOITFrameBuffer = std::make_unique<CFrameBuffer>(WIN_WIDTH, WIN_HEIGHT);
		m_pWBOITFrameBuffer->set(EAttachment::COLOR0, m_pTransparencyColorTex);
		m_pWBOITFrameBuffer->set(EAttachment::COLOR1, m_pAccumulatedTransmittanceTex);
#endif

#ifdef USING_LINKED_LIST_OIT
		m_pLLOITFrameBuffer = std::make_unique<CFrameBuffer>(WIN_WIDTH, WIN_HEIGHT);
		m_pLLOITFrameBuffer->set(EAttachment::COLOR0, m_pTransparencyColorTex);

		m_pListHeadImage = std::make_unique<CImage2D>();
		m_pListHeadImage->createEmpty(WIN_WIDTH, WIN_HEIGHT, GL_R32UI, 0);

		m_pListAtomicCounter = std::make_unique<CAtomicCounterBuffer>(0);

		m_pListNodeBuffer = std::make_unique<CShaderStorageBuffer>(nullptr, MAX_LIST_NODE * sizeof(SListNode), 0);
#endif
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
#ifdef USING_ALL_METHODS
		switch (m_OITMethod)
		{
		case EOITMethod::LINKED_LIST_OIT:
			__renderUsingLinkedListOIT(); break;
		case EOITMethod::MOMENT_BASE_OIT:
			__renderUsingMomentBasedOIT(); break;
		case EOITMethod::WEIGHTED_BLENDED_OIT:
			__renderUsingWeightedBlendedOIT(); break;
		}
#elif defined(USING_MOMENT_BASED_OIT)
		__renderUsingMomentBasedOIT();
#elif defined(USING_LINKED_LIST_OIT)
		__renderUsingLinkedListOIT();
#elif defined(USING_WEIGHTED_BLENDED_OIT)
		__renderUsingWeightedBlendedOIT();
#endif
	}

#ifdef USING_LINKED_LIST_OIT
	void __renderUsingLinkedListOIT()
	{
		m_pLLOITFrameBuffer->bind();
		CRenderer::getInstance()->clear();
		CRenderer::getInstance()->enableCullFace(false);
		CRenderer::getInstance()->setDepthMask(false);
		m_pListHeadImage->clear();

		m_pGenLinkedListShaderProgram->bind();
		m_pListAtomicCounter->reset();

		m_pGenLinkedListShaderProgram->updateUniform1i("uMaxListNode", MAX_LIST_NODE);
		m_pOpaqueDepthTex->bindV(2);
		m_pGenLinkedListShaderProgram->updateUniformTexture("uOpaqueDepthTex", m_pOpaqueDepthTex.get());

		for (auto Model : m_TransparentModels)
		{
			auto Material = m_Model2MaterialMap[Model];
			m_pGenLinkedListShaderProgram->bind();
			m_pGenLinkedListShaderProgram->updateUniform3f("uDiffuseColor", Material.diffuse);
			m_pGenLinkedListShaderProgram->updateUniform3f("uTransmittance", Material.transmittance);
			m_pGenLinkedListShaderProgram->updateUniform1f("uCoverage", Material.coverage);
			CRenderer::getInstance()->draw(*Model, *m_pGenLinkedListShaderProgram);
		}

		CRenderer::getInstance()->setDepthMask(true);

		//pass2: color blending
		m_pColorBlendingShaderProgram->bind();
		CRenderer::getInstance()->drawScreenQuad(*m_pColorBlendingShaderProgram);

		m_pLLOITFrameBuffer->unbind();

		//pass3: merge color
		CRenderer::getInstance()->clear();

		m_pOpaqueColorTex->bindV(0);
		m_pTransparencyColorTex->bindV(1);

		m_pLLOITMergeColorShaderProgram->bind();
		m_pLLOITMergeColorShaderProgram->updateUniformTexture("uOpaqueColorTex", m_pOpaqueColorTex.get());
		m_pLLOITMergeColorShaderProgram->updateUniformTexture("uTransparentColorTex", m_pTransparencyColorTex.get());

		CRenderer::getInstance()->drawScreenQuad(*m_pLLOITMergeColorShaderProgram);
	}
#endif


#ifdef USING_MOMENT_BASED_OIT
	void __renderUsingMomentBasedOIT()
	{
		//pass1: generate moments
		m_pMBOITFrameBuffer1->bind();

		CRenderer::getInstance()->clear();
		CRenderer::getInstance()->enableCullFace(false);
		CRenderer::getInstance()->setDepthMask(false);
		CRenderer::getInstance()->enableBlend(true);
		CRenderer::getInstance()->setBlendFunc(GL_ONE, GL_ONE);

		m_pGenerateMomentShaderProgram->bind();
		m_pOpaqueDepthTex->bindV(2);
		m_pGenerateMomentShaderProgram->updateUniformTexture("uOpaqueDepthTex", m_pOpaqueDepthTex.get());

		for (auto Model : m_TransparentModels)
		{
			auto Material = m_Model2MaterialMap[Model];
			m_pGenerateMomentShaderProgram->bind();
			m_pGenerateMomentShaderProgram->updateUniform1f("uCoverage", Material.coverage);
			CRenderer::getInstance()->draw(*Model, *m_pGenerateMomentShaderProgram);
		}

		CRenderer::getInstance()->setDepthMask(true);
		CRenderer::getInstance()->enableBlend(false);

		m_pMBOITFrameBuffer1->unbind();

		//pass2: reconstruct transmittance
		m_pMBOITFrameBuffer2->bind();

		CRenderer::getInstance()->clear();
		CRenderer::getInstance()->enableCullFace(false);
		CRenderer::getInstance()->setDepthMask(false);
		CRenderer::getInstance()->enableBlend(true);
		CRenderer::getInstance()->setBlendFunc(GL_ONE, GL_ONE);

		m_pReconstructTransmittanceShaderProgram->bind();
		m_pOpaqueDepthTex->bindV(2);
		m_pMomentB0Tex->bindV(3);
		m_pMomentsTex->bindV(4);
		m_pExtraMomentsTex->bindV(5);
		m_pReconstructTransmittanceShaderProgram->updateUniformTexture("uOpaqueDepthTex", m_pOpaqueDepthTex.get());

		for (auto Model : m_TransparentModels)
		{
			auto Material = m_Model2MaterialMap[Model];
			m_pReconstructTransmittanceShaderProgram->bind();
			m_pReconstructTransmittanceShaderProgram->updateUniform3f("uDiffuseColor", Material.diffuse);
			m_pReconstructTransmittanceShaderProgram->updateUniform1f("uCoverage", Material.coverage);
			m_pReconstructTransmittanceShaderProgram->updateUniformTexture("uMomentB0Tex", m_pMomentB0Tex.get());
			m_pReconstructTransmittanceShaderProgram->updateUniformTexture("uMomentsTex", m_pMomentsTex.get());
			m_pReconstructTransmittanceShaderProgram->updateUniformTexture("uExtraMomentsTex", m_pExtraMomentsTex.get());
			m_pReconstructTransmittanceShaderProgram->updateUniform1i("uReconstructionStrategy", m_ReconstructionStrategy);
			CRenderer::getInstance()->draw(*Model, *m_pReconstructTransmittanceShaderProgram);
		}

		CRenderer::getInstance()->setDepthMask(true);
		CRenderer::getInstance()->enableBlend(false);

		m_pMBOITFrameBuffer2->unbind();

		//pass3: merge color
		CRenderer::getInstance()->clear();

		m_pOpaqueColorTex->bindV(0);
		m_pTransparencyColorTex->bindV(1);
		m_pMomentB0Tex->bindV(2);

		m_pMBOITMergeColorShaderProgram->bind();
		m_pMBOITMergeColorShaderProgram->updateUniformTexture("uOpaqueColorTex", m_pOpaqueColorTex.get());
		m_pMBOITMergeColorShaderProgram->updateUniformTexture("uTranslucentColorTex", m_pTransparencyColorTex.get());
		m_pMBOITMergeColorShaderProgram->updateUniformTexture("uMomentB0Tex", m_pMomentB0Tex.get());

		CRenderer::getInstance()->drawScreenQuad(*m_pMBOITMergeColorShaderProgram);
	}
#endif

#ifdef USING_WEIGHTED_BLENDED_OIT
	void __renderUsingWeightedBlendedOIT()
	{
		//pass1: weighted blending
		m_pWBOITFrameBuffer->bind();

		CRenderer::getInstance()->clear();
		CRenderer::getInstance()->enableCullFace(false);
		CRenderer::getInstance()->setDepthMask(false);
		CRenderer::getInstance()->enableBlend(true);
		CRenderer::getInstance()->setBlendFunc(GL_ONE, GL_ONE);

		m_pWeightedBlendingShaderProgram->bind();
		m_pOpaqueDepthTex->bindV(2);
		m_pWeightedBlendingShaderProgram->updateUniformTexture("uOpaqueDepthTex", m_pOpaqueDepthTex.get());
		m_pWeightedBlendingShaderProgram->updateUniform1i("uWeightingStragety", m_WeightingStrategy);

		for (auto Model : m_TransparentModels)
		{
			auto Material = m_Model2MaterialMap[Model];
			m_pWeightedBlendingShaderProgram->bind();
			m_pWeightedBlendingShaderProgram->updateUniform1f("uCoverage", Material.coverage);
			m_pWeightedBlendingShaderProgram->updateUniform3f("uTransmittance", Material.transmittance);
			m_pWeightedBlendingShaderProgram->updateUniform3f("uDiffuseColor", Material.diffuse);
			CRenderer::getInstance()->draw(*Model, *m_pWeightedBlendingShaderProgram);
		}

		CRenderer::getInstance()->setDepthMask(true);
		CRenderer::getInstance()->enableBlend(false);

		m_pWBOITFrameBuffer->unbind();

		//pass2: merge color
		CRenderer::getInstance()->clear();

		m_pOpaqueColorTex->bindV(0);
		m_pTransparencyColorTex->bindV(1);
		m_pAccumulatedTransmittanceTex->bindV(2);

		m_pWBOITMergeColorShaderProgram->bind();
		m_pWBOITMergeColorShaderProgram->updateUniformTexture("uOpaqueColorTex", m_pOpaqueColorTex.get());
		m_pWBOITMergeColorShaderProgram->updateUniformTexture("uAccumulatedTranslucentColorTex", m_pTransparencyColorTex.get());
		m_pWBOITMergeColorShaderProgram->updateUniformTexture("uAccumulatedTransmittanceTex", m_pAccumulatedTransmittanceTex.get());

		CRenderer::getInstance()->drawScreenQuad(*m_pWBOITMergeColorShaderProgram);
	}
#endif

	std::unique_ptr<CSkybox>						m_pSkybox;
	std::vector<std::shared_ptr<CModel>>			m_OpaqueModels;
	std::vector<std::shared_ptr<CModel>>			m_TransparentModels;
	std::map<std::shared_ptr<CModel>, SMaterial>	m_Model2MaterialMap;

	std::unique_ptr<CShaderProgram> m_pOpaqueShaderProgram;
	std::unique_ptr<CFrameBuffer>	m_pOpaqueFrameBuffer;
	std::shared_ptr<CTexture2D>		m_pOpaqueColorTex;
	std::shared_ptr<CTexture2D>		m_pOpaqueDepthTex;
	std::shared_ptr<CTexture2D>		m_pTransparencyColorTex;

#ifdef USING_ALL_METHODS
	EOITMethod m_OITMethod = EOITMethod::LINKED_LIST_OIT;
#endif

#ifdef USING_MOMENT_BASED_OIT
	std::unique_ptr<CShaderProgram> m_pGenerateMomentShaderProgram;
	std::unique_ptr<CShaderProgram> m_pReconstructTransmittanceShaderProgram;
	std::unique_ptr<CShaderProgram> m_pMBOITMergeColorShaderProgram;
	std::unique_ptr<CFrameBuffer>	m_pMBOITFrameBuffer1;
	std::unique_ptr<CFrameBuffer>	m_pMBOITFrameBuffer2;
	std::shared_ptr<CTexture2D>		m_pMomentB0Tex;
	std::shared_ptr<CTexture2D>		m_pMomentsTex;
	std::shared_ptr<CTexture2D>		m_pExtraMomentsTex;

	int m_ReconstructionStrategy = 0;
#endif

#ifdef USING_WEIGHTED_BLENDED_OIT
	std::unique_ptr<CShaderProgram> m_pWeightedBlendingShaderProgram;
	std::unique_ptr<CShaderProgram> m_pWBOITMergeColorShaderProgram;
	std::unique_ptr<CFrameBuffer>	m_pWBOITFrameBuffer;
	std::shared_ptr<CTexture2D>		m_pAccumulatedTransmittanceTex;

	int m_WeightingStrategy = 0;
#endif

#ifdef USING_LINKED_LIST_OIT
	std::unique_ptr<CShaderProgram> m_pGenLinkedListShaderProgram;
	std::unique_ptr<CShaderProgram> m_pColorBlendingShaderProgram;
	std::unique_ptr<CShaderProgram> m_pLLOITMergeColorShaderProgram;
	std::unique_ptr<CFrameBuffer>	m_pLLOITFrameBuffer;

	std::unique_ptr<CShaderStorageBuffer>	m_pListNodeBuffer;
	std::unique_ptr<CImage2D>				m_pListHeadImage;
	std::unique_ptr<CAtomicCounterBuffer>	m_pListAtomicCounter;
#endif
};

int main()
{
	CMyApplication App;
	if (!App.init(SWindowInfo(WIN_WIDTH, WIN_HEIGHT, "Order Independent Transparency Demo"))) return -1;
	App.run();

	return 0;
}