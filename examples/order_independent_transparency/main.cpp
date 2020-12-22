#include <memory>
#include <vector>
#include <map>
#include <cmath>
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
#include "Scene.h"

#ifndef M_PI
#define M_PI 3.14159265358979323f
#endif

#define USING_ALL_METHODS

#ifdef USING_ALL_METHODS
#define USING_MOMENT_BASED_OIT
#define USING_WEIGHTED_BLENDED_OIT
#define USING_LINKED_LIST_OIT
#define USING_FOURIER_OIT
#define USING_WAVELET_OIT
#endif

enum class EOITMethod : unsigned char
{
	LINKED_LIST_OIT = 0,
	MOMENT_BASE_OIT,
	WEIGHTED_BLENDED_OIT,
	FOURIER_OIT,
	WAVELET_OIT
};

using namespace glt;

const int WIN_WIDTH = 1600;
const int WIN_HEIGHT = 900;

#ifdef USING_LINKED_LIST_OIT
const int MAX_LIST_NODE = WIN_WIDTH * WIN_HEIGHT * 64;

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
		//setDisplayStatusHint();

		CFileLocator::getInstance()->addFileSearchPath("../../resource");

		__initShaders();
		__initScene();
		__initTexturesAndBuffers();
		__extraInit();

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
		else if (KeyStatus[GLFW_KEY_F]) m_OITMethod = EOITMethod::FOURIER_OIT;
		else if (KeyStatus[GLFW_KEY_V]) m_OITMethod = EOITMethod::WAVELET_OIT;
#endif

#ifdef USING_WEIGHTED_BLENDED_OIT
		if (KeyStatus[GLFW_KEY_0]) m_WBOITStrategy = 0;
		else if (KeyStatus[GLFW_KEY_1]) m_WBOITStrategy = 1;
#endif

#ifdef USING_LINKED_LIST_OIT
		if (KeyStatus[GLFW_KEY_0]) m_UseThickness = false;
		else if (KeyStatus[GLFW_KEY_1]) m_UseThickness = true;
#endif

#ifdef USING_FOURIER_OIT
		if (KeyStatus[GLFW_KEY_0]) m_FOITStrategy = 0;
		else if (KeyStatus[GLFW_KEY_1]) m_FOITStrategy = 1;
		else if (KeyStatus[GLFW_KEY_2]) m_FOITStrategy = 2;
		else if (KeyStatus[GLFW_KEY_3]) m_FOITStrategy = 3;
#endif

#ifdef USING_WAVELET_OIT
		if (KeyStatus[GLFW_KEY_0]) m_WOITStrategy = 0;
		else if (KeyStatus[GLFW_KEY_1]) m_WOITStrategy = 1;
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

#ifdef USING_FOURIER_OIT
		m_pGenFourierOpacityMapSP = std::make_unique<CShaderProgram>();
		m_pGenFourierOpacityMapSP->addShader("shaders/FOIT_generate_fourier_opacity_map.vert", EShaderType::VERTEX_SHADER);
		m_pGenFourierOpacityMapSP->addShader("shaders/FOIT_generate_fourier_opacity_map.frag", EShaderType::FRAGMENT_SHADER);

		m_pFOITReconstructTransmittanceSP = std::make_unique<CShaderProgram>();
		m_pFOITReconstructTransmittanceSP->addShader("shaders/FOIT_reconstruct_transmittance.vert", EShaderType::VERTEX_SHADER);
		m_pFOITReconstructTransmittanceSP->addShader("shaders/FOIT_reconstruct_transmittance.frag", EShaderType::FRAGMENT_SHADER);

		m_pFOITMergerColorSP = std::make_unique<CShaderProgram>();
		m_pFOITMergerColorSP->addShader("shaders/draw_screen_coord.vert", EShaderType::VERTEX_SHADER);
		m_pFOITMergerColorSP->addShader("shaders/FOIT_merge_color.frag", EShaderType::FRAGMENT_SHADER);
#endif

#ifdef USING_WAVELET_OIT
		m_pGenWaveletOpacityMapSP = std::make_unique<CShaderProgram>();
		m_pGenWaveletOpacityMapSP->addShader("shaders/WOIT_generate_wavelet_opacity_map.vert", EShaderType::VERTEX_SHADER);
		m_pGenWaveletOpacityMapSP->addShader("shaders/WOIT_generate_wavelet_opacity_map.frag", EShaderType::FRAGMENT_SHADER);

		m_pWOITReconstructTransmittanceSP = std::make_unique<CShaderProgram>();
		m_pWOITReconstructTransmittanceSP->addShader("shaders/WOIT_reconstruct_transmittance.vert", EShaderType::VERTEX_SHADER);
		m_pWOITReconstructTransmittanceSP->addShader("shaders/WOIT_reconstruct_transmittance.frag", EShaderType::FRAGMENT_SHADER);

		m_pWOITMergerColorSP = std::make_unique<CShaderProgram>();
		m_pWOITMergerColorSP->addShader("shaders/draw_screen_coord.vert", EShaderType::VERTEX_SHADER);
		m_pWOITMergerColorSP->addShader("shaders/WOIT_merge_color.frag", EShaderType::FRAGMENT_SHADER);
#endif
	}

	void __initScene()
	{
		CCPUTimer timer;
		timer.start();

		auto pCamera = CRenderer::getInstance()->fetchCamera();
		pCamera->setPosition(glm::dvec3(0, 0, 4));
		pCamera->setNearPlane(0.1);
		pCamera->setFarPlane(10.0);
		pCamera->setMoveSpeed(0.01);

		std::vector<std::string> Faces = {
			"textures/skybox/right.jpg",
			"textures/skybox/left.jpg",
			"textures/skybox/top.jpg",
			"textures/skybox/bottom.jpg",
			"textures/skybox/front.jpg",
			"textures/skybox/back.jpg"
		};
		m_pSkybox = std::make_unique<CSkybox>(Faces);

		m_Scene.load("scene_05.json");
		m_OpaqueModels = m_Scene.getModelGroup("opaqueModels");
		m_TransparentModels = m_Scene.getModelGroup("transparentModels");
		for (auto pModel : m_TransparentModels)
		{
			glm::vec4 Params = pModel->getParameters();
			m_Model2MaterialMap[pModel] = SMaterial(glm::vec3(Params.x, Params.y, Params.z), Params.w);
		}

		timer.stop();
		_OUTPUT_EVENT(format("Total time: %f", timer.getElapsedTimeInMS()));
	}

	void __initTexturesAndBuffers()
	{
		m_pOpaqueColorTex = std::make_shared<CTexture2D>();
		m_pOpaqueColorTex->createEmpty(WIN_WIDTH, WIN_HEIGHT, GL_RGB8, GL_CLAMP_TO_BORDER, GL_NEAREST);

		m_pOpaqueDepthTex = std::make_shared<CTexture2D>();
		m_pOpaqueDepthTex->createEmpty(WIN_WIDTH, WIN_HEIGHT, GL_DEPTH_COMPONENT16, GL_CLAMP_TO_BORDER, GL_NEAREST);

		m_pOpaqueFrameBuffer = std::make_unique<CFrameBuffer>(WIN_WIDTH, WIN_HEIGHT, false);
		m_pOpaqueFrameBuffer->set(EAttachment::COLOR0, m_pOpaqueColorTex);
		m_pOpaqueFrameBuffer->set(EAttachment::DEPTH, m_pOpaqueDepthTex);

		m_pTransparencyColorTex = std::make_shared<CTexture2D>();
		m_pTransparencyColorTex->createEmpty(WIN_WIDTH, WIN_HEIGHT, GL_RGBA16F, GL_CLAMP_TO_BORDER, GL_NEAREST);

#ifdef USING_MOMENT_BASED_OIT
		m_pMomentB0Tex = std::make_shared<CTexture2D>();
		m_pMomentB0Tex->createEmpty(WIN_WIDTH, WIN_HEIGHT, GL_R32F, GL_CLAMP_TO_BORDER, GL_NEAREST);

		m_pMomentsImage = std::make_shared<CImage2D>();
		m_pMomentsImage->createEmpty(WIN_WIDTH, WIN_HEIGHT, GL_RGBA32F, 1);

		m_pMBOITFrameBuffer1 = std::make_unique<CFrameBuffer>(WIN_WIDTH, WIN_HEIGHT, false);
		m_pMBOITFrameBuffer1->set(EAttachment::COLOR0, m_pMomentB0Tex);

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

#ifdef USING_FOURIER_OIT
		m_pFourierOpacityMap1 = std::make_shared<CTexture2D>();
		m_pFourierOpacityMap1->createEmpty(WIN_WIDTH, WIN_HEIGHT, GL_RGBA16F, GL_CLAMP_TO_BORDER, GL_NEAREST);

		m_pFourierOpacityMap2 = std::make_shared<CTexture2D>();
		m_pFourierOpacityMap2->createEmpty(WIN_WIDTH, WIN_HEIGHT, GL_RGBA16F, GL_CLAMP_TO_BORDER, GL_NEAREST);

		m_pFourierOpacityMap3 = std::make_shared<CTexture2D>();
		m_pFourierOpacityMap3->createEmpty(WIN_WIDTH, WIN_HEIGHT, GL_RGBA16F, GL_CLAMP_TO_BORDER, GL_NEAREST);

		m_pFourierOpacityMap4 = std::make_shared<CTexture2D>();
		m_pFourierOpacityMap4->createEmpty(WIN_WIDTH, WIN_HEIGHT, GL_RGBA16F, GL_CLAMP_TO_BORDER, GL_NEAREST);

		m_pFOITFrameBuffer1 = std::make_unique<CFrameBuffer>(WIN_WIDTH, WIN_HEIGHT,false);
		m_pFOITFrameBuffer1->set(EAttachment::COLOR0, m_pFourierOpacityMap1);
		m_pFOITFrameBuffer1->set(EAttachment::COLOR1, m_pFourierOpacityMap2);
		m_pFOITFrameBuffer1->set(EAttachment::COLOR2, m_pFourierOpacityMap3);
		m_pFOITFrameBuffer1->set(EAttachment::COLOR3, m_pFourierOpacityMap4);

		m_pFOITFrameBuffer2 = std::make_unique<CFrameBuffer>(WIN_WIDTH, WIN_HEIGHT);
		m_pFOITFrameBuffer2->set(EAttachment::COLOR0, m_pTransparencyColorTex);
#endif

#ifdef USING_WAVELET_OIT
		m_pWaveletOpacityMap1 = std::make_shared<CTexture2D>();
		m_pWaveletOpacityMap1->createEmpty(WIN_WIDTH, WIN_HEIGHT, GL_RGBA16F, GL_CLAMP_TO_BORDER, GL_NEAREST);

		m_pWaveletOpacityMap2 = std::make_shared<CTexture2D>();
		m_pWaveletOpacityMap2->createEmpty(WIN_WIDTH, WIN_HEIGHT, GL_RGBA16F, GL_CLAMP_TO_BORDER, GL_NEAREST);

		m_pWaveletOpacityMap3 = std::make_shared<CTexture2D>();
		m_pWaveletOpacityMap3->createEmpty(WIN_WIDTH, WIN_HEIGHT, GL_RGBA16F, GL_CLAMP_TO_BORDER, GL_NEAREST);

		m_pWaveletOpacityMap4 = std::make_shared<CTexture2D>();
		m_pWaveletOpacityMap4->createEmpty(WIN_WIDTH, WIN_HEIGHT, GL_RGBA16F, GL_CLAMP_TO_BORDER, GL_NEAREST);

		m_pTotalAbsorbanceTex = std::make_shared<CTexture2D>();
		m_pTotalAbsorbanceTex->createEmpty(WIN_WIDTH, WIN_HEIGHT, GL_R16F, GL_CLAMP_TO_BORDER, GL_NEAREST);

		m_pWOITFrameBuffer1 = std::make_unique<CFrameBuffer>(WIN_WIDTH, WIN_HEIGHT, false);
		m_pWOITFrameBuffer1->set(EAttachment::COLOR0, m_pWaveletOpacityMap1);
		m_pWOITFrameBuffer1->set(EAttachment::COLOR1, m_pWaveletOpacityMap2);
		m_pWOITFrameBuffer1->set(EAttachment::COLOR2, m_pWaveletOpacityMap3);
		m_pWOITFrameBuffer1->set(EAttachment::COLOR3, m_pWaveletOpacityMap4);
		m_pWOITFrameBuffer1->set(EAttachment::COLOR4, m_pTotalAbsorbanceTex);

		m_pWOITFrameBuffer2 = std::make_unique<CFrameBuffer>(WIN_WIDTH, WIN_HEIGHT);
		m_pWOITFrameBuffer2->set(EAttachment::COLOR0, m_pTransparencyColorTex);

		m_pPsiIntegralLutTex = std::make_shared<CTexture2D>();
		m_pPsiIntegralLutTex->load16("textures/psi_int.png", GL_CLAMP_TO_BORDER, GL_NEAREST);

		m_pPsiLutTex = std::make_shared<CTexture2D>();
		m_pPsiLutTex->load16("textures/psi.png", GL_CLAMP_TO_BORDER, GL_NEAREST);
#endif

#if defined(USING_LINKED_LIST_OIT) || defined(USING_MOMENT_BASED_OIT)
		m_pClearImageFrameBuffer = std::make_unique<CFrameBuffer>(WIN_WIDTH, WIN_HEIGHT);
#endif
	}

	void __extraInit()
	{
#ifdef USING_MOMENT_BASED_OIT
		float Temp[4];
		__computeWrappingZoneParameters(Temp);
		m_WrappingZoneParameters = glm::vec4(Temp[0], Temp[1], Temp[2], Temp[3]);
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
		if (!m_OpaqueModels.empty()) CRenderer::getInstance()->draw(m_OpaqueModels, *m_pOpaqueShaderProgram);
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
		case EOITMethod::FOURIER_OIT:
			__renderUsingFourierOIT(); break;
		case EOITMethod::WAVELET_OIT:
			__renderUsingWaveletOIT(); break;
		}
#elif defined(USING_MOMENT_BASED_OIT)
		__renderUsingMomentBasedOIT();
#elif defined(USING_LINKED_LIST_OIT)
		__renderUsingLinkedListOIT();
#elif defined(USING_WEIGHTED_BLENDED_OIT)
		__renderUsingWeightedBlendedOIT();
#elif defined(USING_FOURIER_OIT)
		__renderUsingFourierOIT();
#elif defined(USING_WAVELET_OIT)
		__renderUsingWaveletOIT();
#endif
	}

#ifdef USING_LINKED_LIST_OIT
	void __renderUsingLinkedListOIT()
	{
		m_pClearImageFrameBuffer->bind();
		m_pClearImageFrameBuffer->set(EAttachment::COLOR0, m_pListHeadImage);
		CRenderer::getInstance()->clear();
		m_pClearImageFrameBuffer->unbind();

		//pass1: generate linked list
		m_pLLOITFrameBuffer->bind();
		CRenderer::getInstance()->clear();
		CRenderer::getInstance()->enableCullFace(false);
		CRenderer::getInstance()->setDepthMask(false);

		m_pGenLinkedListShaderProgram->bind();
		m_pListAtomicCounter->reset();

		m_pGenLinkedListShaderProgram->updateUniform1i("uMaxListNode", MAX_LIST_NODE);
		m_pOpaqueDepthTex->bindV(2);
		m_pGenLinkedListShaderProgram->updateUniformTexture("uOpaqueDepthTex", m_pOpaqueDepthTex.get());

		auto pCamera = CRenderer::getInstance()->fetchCamera();
		m_pGenLinkedListShaderProgram->updateUniform1f("uNearPlane", pCamera->getNear());
		m_pGenLinkedListShaderProgram->updateUniform1f("uFarPlane", pCamera->getFar());

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
		//CRenderer::getInstance()->memoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);

		//pass2: color blending
		m_pColorBlendingShaderProgram->bind();
		m_pColorBlendingShaderProgram->updateUniform1i("uUseThickness", m_UseThickness);
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
		m_pClearImageFrameBuffer->bind();
		m_pClearImageFrameBuffer->set(EAttachment::COLOR0, m_pMomentsImage);
		CRenderer::getInstance()->clear();
		m_pClearImageFrameBuffer->unbind();

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
		m_pGenerateMomentShaderProgram->updateUniform4f("uWrappingZoneParameters", m_WrappingZoneParameters);

		auto pCamera = CRenderer::getInstance()->fetchCamera();
		m_pGenerateMomentShaderProgram->updateUniform1f("uNearPlane", pCamera->getNear());
		m_pGenerateMomentShaderProgram->updateUniform1f("uFarPlane", pCamera->getFar());

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
		m_pReconstructTransmittanceShaderProgram->updateUniformTexture("uOpaqueDepthTex", m_pOpaqueDepthTex.get());
		m_pReconstructTransmittanceShaderProgram->updateUniformTexture("uMomentB0Tex", m_pMomentB0Tex.get());
		m_pReconstructTransmittanceShaderProgram->updateUniform4f("uWrappingZoneParameters", m_WrappingZoneParameters);

		m_pReconstructTransmittanceShaderProgram->updateUniform1f("uNearPlane", pCamera->getNear());
		m_pReconstructTransmittanceShaderProgram->updateUniform1f("uFarPlane", pCamera->getFar());

		for (auto Model : m_TransparentModels)
		{
			auto Material = m_Model2MaterialMap[Model];
			m_pReconstructTransmittanceShaderProgram->bind();
			m_pReconstructTransmittanceShaderProgram->updateUniform3f("uDiffuseColor", Material.diffuse);
			m_pReconstructTransmittanceShaderProgram->updateUniform1f("uCoverage", Material.coverage);
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

	//code from http://momentsingraphics.de/MissingTMBOITCode.html
	float __circleToParameter(float vAngle, float* voMaxParameter = nullptr) 
	{
		float x = std::cos(vAngle);
		float y = std::sin(vAngle);
		float result = std::abs(y) - std::abs(x);
		result = (x < 0.0f) ? (2.0f - result) : result;
		result = (y < 0.0f) ? (6.0f - result) : result;
		result += (vAngle >= 2.0f * M_PI) ? 8.0f : 0.0f;
		if (voMaxParameter) {
			(*voMaxParameter) = 7.0f;
		}
		return result;
	}

	void __computeWrappingZoneParameters(float voWrappingZoneParameters[4], float vNewWrappingZoneAngle = 0.1f * M_PI) 
	{
		voWrappingZoneParameters[0] = vNewWrappingZoneAngle;
		voWrappingZoneParameters[1] = M_PI - 0.5f * vNewWrappingZoneAngle;
		if (vNewWrappingZoneAngle <= 0.0f) {
			voWrappingZoneParameters[2] = 0.0f;
			voWrappingZoneParameters[3] = 0.0f;
		}
		else {
			float zone_end_parameter;
			float zone_begin_parameter = __circleToParameter(2.0f * M_PI - vNewWrappingZoneAngle, &zone_end_parameter);
			voWrappingZoneParameters[2] = 1.0f / (zone_end_parameter - zone_begin_parameter);
			voWrappingZoneParameters[3] = 1.0f - zone_end_parameter * voWrappingZoneParameters[2];
		}
	}
#endif

#ifdef USING_FOURIER_OIT
	void __renderUsingFourierOIT()
	{
		int FOITCoeffNum = 0;
		switch (m_FOITStrategy)
		{
		case 0:
			FOITCoeffNum = 15;
			break;
		case 1:
			FOITCoeffNum = 11;
			break;
		case 2:
			FOITCoeffNum = 7;
			break;
		case 3:
			FOITCoeffNum = 3;
		}

		//pass1: generate fourier opacity map
		m_pFOITFrameBuffer1->bind();

		CRenderer::getInstance()->clear();
		CRenderer::getInstance()->enableCullFace(false);
		CRenderer::getInstance()->setDepthMask(false);
		CRenderer::getInstance()->enableBlend(true);
		CRenderer::getInstance()->setBlendFunc(GL_ONE, GL_ONE);

		m_pGenFourierOpacityMapSP->bind();
		m_pOpaqueDepthTex->bindV(2);
		m_pGenFourierOpacityMapSP->updateUniformTexture("uOpaqueDepthTex", m_pOpaqueDepthTex.get());

		auto pCamera = CRenderer::getInstance()->fetchCamera();
		m_pGenFourierOpacityMapSP->updateUniform1f("uNearPlane", pCamera->getNear());
		m_pGenFourierOpacityMapSP->updateUniform1f("uFarPlane", pCamera->getFar());

		m_pGenFourierOpacityMapSP->updateUniform1i("uFOITCoeffNum", FOITCoeffNum);

		for (auto Model : m_TransparentModels)
		{
			auto Material = m_Model2MaterialMap[Model];
			m_pGenFourierOpacityMapSP->bind();
			m_pGenFourierOpacityMapSP->updateUniform1f("uCoverage", Material.coverage);
			CRenderer::getInstance()->draw(*Model, *m_pGenFourierOpacityMapSP);
		}

		CRenderer::getInstance()->setDepthMask(true);
		CRenderer::getInstance()->enableBlend(false);

		m_pFOITFrameBuffer1->unbind();

		//pass2: reconstruct transmittance
		m_pFOITFrameBuffer2->bind();

		CRenderer::getInstance()->clear();
		CRenderer::getInstance()->enableCullFace(false);
		CRenderer::getInstance()->setDepthMask(false);
		CRenderer::getInstance()->enableBlend(true);
		CRenderer::getInstance()->setBlendFunc(GL_ONE, GL_ONE);

		m_pFOITReconstructTransmittanceSP->bind();
		m_pOpaqueDepthTex->bindV(2);
		m_pFourierOpacityMap1->bindV(3);
		m_pFourierOpacityMap2->bindV(4);
		m_pFourierOpacityMap3->bindV(5);
		m_pFourierOpacityMap4->bindV(6);
		m_pFOITReconstructTransmittanceSP->updateUniformTexture("uOpaqueDepthTex", m_pOpaqueDepthTex.get());
		m_pFOITReconstructTransmittanceSP->updateUniformTexture("uFourierOpacityMap1", m_pFourierOpacityMap1.get());
		m_pFOITReconstructTransmittanceSP->updateUniformTexture("uFourierOpacityMap2", m_pFourierOpacityMap2.get());
		m_pFOITReconstructTransmittanceSP->updateUniformTexture("uFourierOpacityMap3", m_pFourierOpacityMap3.get());
		m_pFOITReconstructTransmittanceSP->updateUniformTexture("uFourierOpacityMap4", m_pFourierOpacityMap4.get());

		m_pFOITReconstructTransmittanceSP->updateUniform1f("uNearPlane", pCamera->getNear());
		m_pFOITReconstructTransmittanceSP->updateUniform1f("uFarPlane", pCamera->getFar());

		m_pFOITReconstructTransmittanceSP->updateUniform1i("uFOITCoeffNum", FOITCoeffNum);

		for (auto Model : m_TransparentModels)
		{
			auto Material = m_Model2MaterialMap[Model];
			m_pFOITReconstructTransmittanceSP->bind();
			m_pFOITReconstructTransmittanceSP->updateUniform3f("uDiffuseColor", Material.diffuse);
			m_pFOITReconstructTransmittanceSP->updateUniform1f("uCoverage", Material.coverage);
			CRenderer::getInstance()->draw(*Model, *m_pFOITReconstructTransmittanceSP);
		}

		CRenderer::getInstance()->setDepthMask(true);
		CRenderer::getInstance()->enableBlend(false);

		m_pFOITFrameBuffer2->unbind();

		//pass3: merge color
		CRenderer::getInstance()->clear();

		m_pOpaqueColorTex->bindV(0);
		m_pTransparencyColorTex->bindV(1);
		m_pFourierOpacityMap1->bindV(2);

		m_pFOITMergerColorSP->bind();
		m_pFOITMergerColorSP->updateUniformTexture("uOpaqueColorTex", m_pOpaqueColorTex.get());
		m_pFOITMergerColorSP->updateUniformTexture("uTranslucentColorTex", m_pTransparencyColorTex.get());
		m_pFOITMergerColorSP->updateUniformTexture("uFourierOpacityMap1", m_pFourierOpacityMap1.get());

		CRenderer::getInstance()->drawScreenQuad(*m_pFOITMergerColorSP);
	}
#endif

#ifdef USING_WAVELET_OIT
	void __renderUsingWaveletOIT()
	{
		int WOITCoeffNum = 0;
		switch (m_WOITStrategy)
		{
		case 0:
			WOITCoeffNum = 8;
			break;
		case 1:
			WOITCoeffNum = 16;
			break;
		}

		//pass1: generate wavelet opacity map
		m_pWOITFrameBuffer1->bind();

		CRenderer::getInstance()->clear();
		CRenderer::getInstance()->enableCullFace(false);
		CRenderer::getInstance()->setDepthMask(false);
		CRenderer::getInstance()->enableBlend(true);
		CRenderer::getInstance()->setBlendFunc(GL_ONE, GL_ONE);

		m_pGenWaveletOpacityMapSP->bind();
		m_pOpaqueDepthTex->bindV(2);
		m_pGenWaveletOpacityMapSP->updateUniformTexture("uOpaqueDepthTex", m_pOpaqueDepthTex.get());
		m_pPsiLutTex->bindV(3);
		m_pGenWaveletOpacityMapSP->updateUniformTexture("uPsiLutTex", m_pPsiLutTex.get());

		auto pCamera = CRenderer::getInstance()->fetchCamera();
		m_pGenWaveletOpacityMapSP->updateUniform1f("uNearPlane", pCamera->getNear());
		m_pGenWaveletOpacityMapSP->updateUniform1f("uFarPlane", pCamera->getFar());

		m_pGenWaveletOpacityMapSP->updateUniform1i("uWOITCoeffNum", WOITCoeffNum);

		for (auto Model : m_TransparentModels)
		{
			auto Material = m_Model2MaterialMap[Model];
			m_pGenWaveletOpacityMapSP->bind();
			m_pGenWaveletOpacityMapSP->updateUniform1f("uCoverage", Material.coverage);
			CRenderer::getInstance()->draw(*Model, *m_pGenWaveletOpacityMapSP);
		}

		CRenderer::getInstance()->setDepthMask(true);
		CRenderer::getInstance()->enableBlend(false);

		m_pWOITFrameBuffer1->unbind();

		//pass2: reconstruct transmittance
		m_pWOITFrameBuffer2->bind();

		CRenderer::getInstance()->clear();
		CRenderer::getInstance()->enableCullFace(false);
		CRenderer::getInstance()->setDepthMask(false);
		CRenderer::getInstance()->enableBlend(true);
		CRenderer::getInstance()->setBlendFunc(GL_ONE, GL_ONE);

		m_pWOITReconstructTransmittanceSP->bind();
		m_pOpaqueDepthTex->bindV(2);
		m_pWaveletOpacityMap1->bindV(3);
		m_pWaveletOpacityMap2->bindV(4);
		m_pWaveletOpacityMap3->bindV(5);
		m_pWaveletOpacityMap4->bindV(6);
		m_pPsiIntegralLutTex->bindV(7);
		m_pWOITReconstructTransmittanceSP->updateUniformTexture("uOpaqueDepthTex", m_pOpaqueDepthTex.get());
		m_pWOITReconstructTransmittanceSP->updateUniformTexture("uWaveletOpacityMap1", m_pWaveletOpacityMap1.get());
		m_pWOITReconstructTransmittanceSP->updateUniformTexture("uWaveletOpacityMap2", m_pWaveletOpacityMap2.get());
		m_pWOITReconstructTransmittanceSP->updateUniformTexture("uWaveletOpacityMap3", m_pWaveletOpacityMap3.get());
		m_pWOITReconstructTransmittanceSP->updateUniformTexture("uWaveletOpacityMap4", m_pWaveletOpacityMap4.get());
		m_pWOITReconstructTransmittanceSP->updateUniformTexture("uPsiIntegralLutTex", m_pPsiIntegralLutTex.get());

		m_pWOITReconstructTransmittanceSP->updateUniform1f("uNearPlane", pCamera->getNear());
		m_pWOITReconstructTransmittanceSP->updateUniform1f("uFarPlane", pCamera->getFar());

		m_pWOITReconstructTransmittanceSP->updateUniform1i("uWOITCoeffNum", WOITCoeffNum);

		for (auto Model : m_TransparentModels)
		{
			auto Material = m_Model2MaterialMap[Model];
			m_pWOITReconstructTransmittanceSP->bind();
			m_pWOITReconstructTransmittanceSP->updateUniform3f("uDiffuseColor", Material.diffuse);
			m_pWOITReconstructTransmittanceSP->updateUniform1f("uCoverage", Material.coverage);
			CRenderer::getInstance()->draw(*Model, *m_pWOITReconstructTransmittanceSP);
		}

		CRenderer::getInstance()->setDepthMask(true);
		CRenderer::getInstance()->enableBlend(false);

		m_pWOITFrameBuffer2->unbind();

		//pass3: merge color
		CRenderer::getInstance()->clear();

		m_pOpaqueColorTex->bindV(0);
		m_pTransparencyColorTex->bindV(1);
		m_pTotalAbsorbanceTex->bindV(2);

		m_pWOITMergerColorSP->bind();
		m_pWOITMergerColorSP->updateUniformTexture("uOpaqueColorTex", m_pOpaqueColorTex.get());
		m_pWOITMergerColorSP->updateUniformTexture("uTranslucentColorTex", m_pTransparencyColorTex.get());
		m_pWOITMergerColorSP->updateUniformTexture("uTotalAbsorbanceTex", m_pTotalAbsorbanceTex.get());

		CRenderer::getInstance()->drawScreenQuad(*m_pWOITMergerColorSP);
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
		m_pWeightedBlendingShaderProgram->updateUniform1i("uWeightingStragety", m_WBOITStrategy);

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
	CScene m_Scene;

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
	std::shared_ptr<CImage2D>		m_pMomentsImage;
	glm::vec4	m_WrappingZoneParameters;
#endif

#ifdef USING_WEIGHTED_BLENDED_OIT
	std::unique_ptr<CShaderProgram> m_pWeightedBlendingShaderProgram;
	std::unique_ptr<CShaderProgram> m_pWBOITMergeColorShaderProgram;
	std::unique_ptr<CFrameBuffer>	m_pWBOITFrameBuffer;
	std::shared_ptr<CTexture2D>		m_pAccumulatedTransmittanceTex;

	int m_WBOITStrategy = 0;
#endif

#ifdef USING_LINKED_LIST_OIT
	std::unique_ptr<CShaderProgram> m_pGenLinkedListShaderProgram;
	std::unique_ptr<CShaderProgram> m_pColorBlendingShaderProgram;
	std::unique_ptr<CShaderProgram> m_pLLOITMergeColorShaderProgram;
	std::unique_ptr<CFrameBuffer>	m_pLLOITFrameBuffer;

	std::unique_ptr<CShaderStorageBuffer>	m_pListNodeBuffer;
	std::shared_ptr<CImage2D>				m_pListHeadImage;
	std::unique_ptr<CAtomicCounterBuffer>	m_pListAtomicCounter;

	bool m_UseThickness = false;
#endif

#if defined(USING_LINKED_LIST_OIT) || defined(USING_MOMENT_BASED_OIT)
	std::unique_ptr<CFrameBuffer> m_pClearImageFrameBuffer;
#endif

#ifdef USING_FOURIER_OIT
	std::unique_ptr<CShaderProgram> m_pGenFourierOpacityMapSP;
	std::unique_ptr<CShaderProgram> m_pFOITReconstructTransmittanceSP;
	std::unique_ptr<CShaderProgram> m_pFOITMergerColorSP;
	std::unique_ptr<CFrameBuffer>	m_pFOITFrameBuffer1;
	std::unique_ptr<CFrameBuffer>	m_pFOITFrameBuffer2;
	std::shared_ptr<CTexture2D>		m_pFourierOpacityMap1;
	std::shared_ptr<CTexture2D>		m_pFourierOpacityMap2;
	std::shared_ptr<CTexture2D>		m_pFourierOpacityMap3;
	std::shared_ptr<CTexture2D>		m_pFourierOpacityMap4;

	int m_FOITStrategy = 0;
#endif

#ifdef USING_WAVELET_OIT
	std::unique_ptr<CShaderProgram> m_pGenWaveletOpacityMapSP;
	std::unique_ptr<CShaderProgram> m_pWOITReconstructTransmittanceSP;
	std::unique_ptr<CShaderProgram> m_pWOITMergerColorSP;
	std::unique_ptr<CFrameBuffer>	m_pWOITFrameBuffer1;
	std::unique_ptr<CFrameBuffer>	m_pWOITFrameBuffer2;
	std::shared_ptr<CTexture2D>		m_pWaveletOpacityMap1;
	std::shared_ptr<CTexture2D>		m_pWaveletOpacityMap2;
	std::shared_ptr<CTexture2D>		m_pWaveletOpacityMap3;
	std::shared_ptr<CTexture2D>		m_pWaveletOpacityMap4;
	std::shared_ptr<CTexture2D>		m_pPsiLutTex;
	std::shared_ptr<CTexture2D>		m_pPsiIntegralLutTex;
	std::shared_ptr<CTexture2D>		m_pTotalAbsorbanceTex;

	int m_WOITStrategy = 0;
#endif
};

int main()
{
	CMyApplication App;
	if (!App.init(SWindowInfo(WIN_WIDTH, WIN_HEIGHT, "Order Independent Transparency Demo"))) return -1;
	App.run();

	return 0;
}