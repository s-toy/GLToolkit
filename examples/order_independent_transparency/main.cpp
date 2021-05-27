#include <memory>
#include <vector>
#include <map>
#include <cmath>
#include <fstream>
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
#include "shaders/global_macro.h"

#define USING_WAVELET_OIT

#ifdef USING_ALL_METHODS
#define USING_MOMENT_BASED_OIT
#define USING_WEIGHTED_BLENDED_OIT
#define USING_LINKED_LIST_OIT
#define USING_WAVELET_OIT
#endif

const float CAMERA_MOVE_SPEED = 0.005;
const bool DISPLAY_FPS = true;
const glm::dvec3 DEFAULT_CAMERA_POS = glm::dvec3(0, 0, 3);
const std::string SCENE_NAME = "bmw.json";
const std::string LUT_INT_NAME = "textures/db2_psi_int_n16_j43_s20.png";
const std::string LUT_NAME = "textures/db2_psi_n16_j43_s20.png";

enum class EOITMethod : unsigned char
{
	LINKED_LIST_OIT = 0,
	MOMENT_BASE_OIT,
	WEIGHTED_BLENDED_OIT,
	FOURIER_OIT,
	WAVELET_OIT
};

using namespace glt;

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

#define MAX_NUM_AABB 32

struct AABBs 
{
	glm::vec3 minVertices[MAX_NUM_AABB] = {};
	glm::vec3 maxVertices[MAX_NUM_AABB] = {};
};

class CMyApplication : public CApplicationBase
{
protected:
	bool _initV() override
	{
		if (DISPLAY_FPS) setDisplayStatusHint();

		CFileLocator::getInstance()->addFileSearchPath("../../resource");

		m_pRenderer = CRenderer::getInstance();
		assert(m_pRenderer != nullptr);

		__initShaders();
		__initScene();
		__initTexturesAndBuffers();
		__extraInit();

		return true;
	}

	void _renderV() override
	{
		__drawOpaqueObjects();

		m_pRenderer->pushEvent("Draw Transparent Objects");
		__drawTransparentObjects();
		m_pRenderer->popEvent();
	}

	void _updateV() override
	{
		auto pCamera = CRenderer::getInstance()->fetchCamera();
		std::cout << pCamera->getPosition().x << ", " << pCamera->getPosition().y << ", " << pCamera->getPosition().z << std::endl;

		auto KeyStatus = CInputManager::getInstance()->getKeyStatus();

#ifdef USING_ALL_METHODS
		if (KeyStatus[GLFW_KEY_L]) m_OITMethod = EOITMethod::LINKED_LIST_OIT;
		else if (KeyStatus[GLFW_KEY_M]) m_OITMethod = EOITMethod::MOMENT_BASE_OIT;
		else if (KeyStatus[GLFW_KEY_B]) m_OITMethod = EOITMethod::WEIGHTED_BLENDED_OIT;
		else if (KeyStatus[GLFW_KEY_F]) m_OITMethod = EOITMethod::FOURIER_OIT;
		else if (KeyStatus[GLFW_KEY_V]) m_OITMethod = EOITMethod::WAVELET_OIT;
#endif

#ifdef USING_WEIGHTED_BLENDED_OIT
#ifdef USING_ALL_METHODS
		if (m_OITMethod == EOITMethod::WEIGHTED_BLENDED_OIT)
#endif
		{
			if (KeyStatus[GLFW_KEY_0]) m_WBOITStrategy = 0;
			else if (KeyStatus[GLFW_KEY_1]) m_WBOITStrategy = 1;
		}
#endif

#ifdef USING_LINKED_LIST_OIT
#ifdef USING_ALL_METHODS
		if (m_OITMethod == EOITMethod::LINKED_LIST_OIT)
#endif
		{
			if (KeyStatus[GLFW_KEY_0]) m_UseThickness = false;
			else if (KeyStatus[GLFW_KEY_1]) m_UseThickness = true;
		}
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

		m_pComputeDepthRemapTexSP = std::make_unique<CShaderProgram>();
		m_pComputeDepthRemapTexSP->addShader("shaders/draw_screen_coord.vert", EShaderType::VERTEX_SHADER);
		m_pComputeDepthRemapTexSP->addShader("shaders/WOIT_compute_depth_remap_texture.frag", EShaderType::FRAGMENT_SHADER);

#endif //USING_WAVELET_OIT
		}

	void __initScene()
	{
		//std::vector<std::string> Faces = {
		//	"textures/skybox/right.jpg",
		//	"textures/skybox/left.jpg",
		//	"textures/skybox/top.jpg",
		//	"textures/skybox/bottom.jpg",
		//	"textures/skybox/front.jpg",
		//	"textures/skybox/back.jpg"
		//};

		std::vector<std::string> Faces = {
			"textures/meadow/meadow-NX.jpg",
			"textures/meadow/meadow-PX.jpg",
			"textures/meadow/meadow-PY.jpg",
			"textures/meadow/meadow-NY.jpg",
			"textures/meadow/meadow-PZ.jpg",
			"textures/meadow/meadow-NZ.jpg"
		};

		m_pSkybox = std::make_unique<CSkybox>(Faces);

		//m_Scene.load("scene_05.json");
		m_Scene.load(SCENE_NAME);
		m_OpaqueModels = m_Scene.getModelGroup("opaqueModels");
		m_TransparentModels = m_Scene.getModelGroup("transparentModels");
		for (auto pModel : m_TransparentModels)
		{
			glm::vec4 Params = pModel->getParameters();
			m_Model2MaterialMap[pModel] = SMaterial(glm::vec3(Params.x, Params.y, Params.z), Params.w);
		}

		auto pCamera = CRenderer::getInstance()->fetchCamera();
		pCamera->setNearPlane(NEAR_PLANE);
		pCamera->setFarPlane(FAR_PLANE);
		pCamera->setMoveSpeed(CAMERA_MOVE_SPEED);

		if (m_Scene.hasCameraPos())
			pCamera->setPosition(m_Scene.getCameraPos());
		else
			pCamera->setPosition(DEFAULT_CAMERA_POS);
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
		m_pTransparencyColorTex->createEmpty(TRANSPARENT_TEX_WIDTH, TRANSPARENT_TEX_HEIGHT, GL_RGBA16F, GL_CLAMP_TO_EDGE, GL_LINEAR);

#ifdef USING_MOMENT_BASED_OIT
		m_pMomentB0Map = std::make_shared<CTexture2D>();
		m_pMomentB0Map->createEmpty(WIN_WIDTH, WIN_HEIGHT, GL_R32F, GL_CLAMP_TO_BORDER, GL_NEAREST);

		m_pMomentsMap1 = std::make_shared<CTexture2D>();
		m_pMomentsMap1->createEmpty(WIN_WIDTH, WIN_HEIGHT, GL_RGBA32F, GL_CLAMP_TO_BORDER, GL_NEAREST);

		m_pMomentsMap2 = std::make_shared<CTexture2D>();
		m_pMomentsMap2->createEmpty(WIN_WIDTH, WIN_HEIGHT, GL_RGBA32F, GL_CLAMP_TO_BORDER, GL_NEAREST);

		m_pMBOITFrameBuffer1 = std::make_unique<CFrameBuffer>(WIN_WIDTH, WIN_HEIGHT, false);
		m_pMBOITFrameBuffer1->set(EAttachment::COLOR0, m_pMomentB0Map);
		m_pMBOITFrameBuffer1->set(EAttachment::COLOR1, m_pMomentsMap1);
		m_pMBOITFrameBuffer1->set(EAttachment::COLOR2, m_pMomentsMap2);

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

#ifdef USING_WAVELET_OIT

#ifdef WOIT_ENABLE_QUANTIZATION
		m_pWaveletOpacityMap1 = std::make_shared<CImage2D>();
		m_pWaveletOpacityMap1->createEmpty(WIN_WIDTH, WIN_HEIGHT, GL_RGBA8UI, 0, GL_CLAMP_TO_BORDER, GL_NEAREST);

		m_pWaveletOpacityMap2 = std::make_shared<CImage2D>();
		m_pWaveletOpacityMap2->createEmpty(WIN_WIDTH, WIN_HEIGHT, GL_RGBA8UI, 1, GL_CLAMP_TO_BORDER, GL_NEAREST);

		m_pWaveletOpacityMap3 = std::make_shared<CImage2D>();
		m_pWaveletOpacityMap3->createEmpty(WIN_WIDTH, WIN_HEIGHT, GL_RGBA8UI, 2, GL_CLAMP_TO_BORDER, GL_NEAREST);

		m_pWaveletOpacityMap4 = std::make_shared<CImage2D>();
		m_pWaveletOpacityMap4->createEmpty(WIN_WIDTH, WIN_HEIGHT, GL_RGBA8UI, 3, GL_CLAMP_TO_BORDER, GL_NEAREST);
#else
		m_pWaveletOpacityMap1 = std::make_shared<CTexture2D>();
		m_pWaveletOpacityMap1->createEmpty(COEFF_MAP_WIDTH, COEFF_MAP_HEIGHT, GL_RGBA16F, GL_CLAMP_TO_EDGE, GL_LINEAR);

		m_pWaveletOpacityMap2 = std::make_shared<CTexture2D>();
		m_pWaveletOpacityMap2->createEmpty(COEFF_MAP_WIDTH, COEFF_MAP_HEIGHT, GL_RGBA16F, GL_CLAMP_TO_EDGE, GL_LINEAR);

		m_pWaveletOpacityMap3 = std::make_shared<CTexture2D>();
		m_pWaveletOpacityMap3->createEmpty(COEFF_MAP_WIDTH, COEFF_MAP_HEIGHT, GL_RGBA16F, GL_CLAMP_TO_EDGE, GL_LINEAR);

		m_pWaveletOpacityMap4 = std::make_shared<CTexture2D>();
		m_pWaveletOpacityMap4->createEmpty(COEFF_MAP_WIDTH, COEFF_MAP_HEIGHT, GL_RGBA16F, GL_CLAMP_TO_EDGE, GL_LINEAR);
#endif

		m_pTotalAbsorbanceTex = std::make_shared<CTexture2D>();
		m_pTotalAbsorbanceTex->createEmpty(TRANSPARENT_TEX_WIDTH, TRANSPARENT_TEX_HEIGHT, GL_R16F, GL_CLAMP_TO_EDGE, GL_LINEAR);

		m_pDepthRemapTex = std::make_shared<CTexture2D>();
		m_pDepthRemapTex->createEmpty(DEPTH_REMAP_TEX_WIDTH, DEPTH_REMAP_TEX_HEIGHT, GL_RG16F, GL_CLAMP_TO_BORDER, GL_LINEAR_MIPMAP_LINEAR, true);

		m_pComputeDepthRemapTexFrameBuffer = std::make_unique<CFrameBuffer>(DEPTH_REMAP_TEX_WIDTH, DEPTH_REMAP_TEX_HEIGHT, false); // TODO
		m_pComputeDepthRemapTexFrameBuffer->set(EAttachment::COLOR0, m_pDepthRemapTex);

		m_pWOITProjectionFrameBuffer = std::make_unique<CFrameBuffer>(COEFF_MAP_WIDTH, COEFF_MAP_HEIGHT, false);
#ifndef WOIT_ENABLE_QUANTIZATION
		m_pWOITProjectionFrameBuffer->set(EAttachment::COLOR0, m_pWaveletOpacityMap1);
		m_pWOITProjectionFrameBuffer->set(EAttachment::COLOR1, m_pWaveletOpacityMap2);
		m_pWOITProjectionFrameBuffer->set(EAttachment::COLOR2, m_pWaveletOpacityMap3);
		m_pWOITProjectionFrameBuffer->set(EAttachment::COLOR3, m_pWaveletOpacityMap4);
#endif

		m_pWOITReconstructionFrameBuffer = std::make_unique<CFrameBuffer>(TRANSPARENT_TEX_WIDTH, TRANSPARENT_TEX_HEIGHT);
		m_pWOITReconstructionFrameBuffer->set(EAttachment::COLOR0, m_pTransparencyColorTex);
		m_pWOITReconstructionFrameBuffer->set(EAttachment::COLOR1, m_pTotalAbsorbanceTex);

		m_pClearImageFrameBuffer = std::make_unique<CFrameBuffer>(WIN_WIDTH, WIN_HEIGHT);

		m_pPsiIntegralLutTex = std::make_shared<CTexture2D>();
		m_pPsiIntegralLutTex->load16(LUT_INT_NAME.c_str(), GL_CLAMP_TO_BORDER, GL_NEAREST);

		m_pPsiLutTex = std::make_shared<CTexture2D>();
		m_pPsiLutTex->load16(LUT_NAME.c_str(), GL_CLAMP_TO_BORDER, GL_NEAREST);
#endif

#if defined(USING_LINKED_LIST_OIT) || defined(USING_MOMENT_BASED_OIT) || defined(USING_WAVELET_OIT)
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
		m_pRenderer->pushEvent("Draw Skybox");
		m_pOpaqueFrameBuffer->bind();
		CRenderer::getInstance()->clear();
		CRenderer::getInstance()->enableCullFace(true);
		if (m_pSkybox) CRenderer::getInstance()->drawSkybox(*m_pSkybox, 0);
		m_pRenderer->popEvent();

		m_pRenderer->pushEvent("Draw Opaque Objects");
		if (!m_OpaqueModels.empty()) CRenderer::getInstance()->draw(m_OpaqueModels, *m_pOpaqueShaderProgram);
		m_pOpaqueFrameBuffer->unbind();
		m_pRenderer->popEvent();
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
		case EOITMethod::WAVELET_OIT:
			__renderUsingWaveletOIT(); break;
		}
#elif defined(USING_MOMENT_BASED_OIT)
		__renderUsingMomentBasedOIT();
#elif defined(USING_LINKED_LIST_OIT)
		__renderUsingLinkedListOIT();
#elif defined(USING_WEIGHTED_BLENDED_OIT)
		__renderUsingWeightedBlendedOIT();
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
		//m_pClearImageFrameBuffer->bind();
		//m_pClearImageFrameBuffer->set(EAttachment::COLOR0, m_pMomentsMap1);
		//CRenderer::getInstance()->clear();
		//m_pClearImageFrameBuffer->unbind();

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
		m_pMomentB0Map->bindV(3);
		m_pMomentsMap1->bindV(4);
		m_pMomentsMap2->bindV(5);
		m_pReconstructTransmittanceShaderProgram->updateUniformTexture("uOpaqueDepthTex", m_pOpaqueDepthTex.get());
		m_pReconstructTransmittanceShaderProgram->updateUniformTexture("uMomentB0Map", m_pMomentB0Map.get());
		m_pReconstructTransmittanceShaderProgram->updateUniformTexture("uMomentsMap1", m_pMomentsMap1.get());
		m_pReconstructTransmittanceShaderProgram->updateUniformTexture("uMomentsMap2", m_pMomentsMap2.get());
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
		m_pMomentB0Map->bindV(2);

		m_pMBOITMergeColorShaderProgram->bind();
		m_pMBOITMergeColorShaderProgram->updateUniformTexture("uOpaqueColorTex", m_pOpaqueColorTex.get());
		m_pMBOITMergeColorShaderProgram->updateUniformTexture("uTranslucentColorTex", m_pTransparencyColorTex.get());
		m_pMBOITMergeColorShaderProgram->updateUniformTexture("uMomentB0Tex", m_pMomentB0Map.get());

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

#ifdef USING_WAVELET_OIT
	void __renderUsingWaveletOIT()
	{
		auto pCamera = CRenderer::getInstance()->fetchCamera();

		CRenderer::getInstance()->setDepthMask(true);

#ifdef WOIT_ENABLE_QUANTIZATION
		m_pClearImageFrameBuffer->bind();
		m_pClearImageFrameBuffer->set(EAttachment::COLOR0, m_pWaveletOpacityMap1);
		m_pClearImageFrameBuffer->set(EAttachment::COLOR1, m_pWaveletOpacityMap2);
		m_pClearImageFrameBuffer->set(EAttachment::COLOR2, m_pWaveletOpacityMap3);
		m_pClearImageFrameBuffer->set(EAttachment::COLOR3, m_pWaveletOpacityMap4);
		CRenderer::getInstance()->clear();
		m_pClearImageFrameBuffer->unbind();
#endif

		m_pRenderer->pushEvent("Compute Depth Remapping Texture");
#ifdef ENABLE_DEPTH_REMAPPING
		m_pComputeDepthRemapTexFrameBuffer->bind();
		glViewport(0, 0, DEPTH_REMAP_TEX_WIDTH, DEPTH_REMAP_TEX_HEIGHT);
		CRenderer::getInstance()->setClearColor(0, 1, 0, 0);
		CRenderer::getInstance()->clear();

		AABBs aabbs;

		for (int i = 0; i < m_TransparentModels.size(); ++i)
		{
			SAABB aabb = m_TransparentModels[i]->getAABB();

			auto ModelMatrix = glm::translate(glm::mat4(1.0), m_TransparentModels[i]->getPosition());
			ModelMatrix = glm::rotate(ModelMatrix, m_TransparentModels[i]->getRotation().w, glm::vec3(m_TransparentModels[i]->getRotation()));
			ModelMatrix = glm::scale(ModelMatrix, m_TransparentModels[i]->getScale());

			aabb.Min = ModelMatrix * glm::vec4(aabb.Min, 1);
			aabb.Max = ModelMatrix * glm::vec4(aabb.Max, 1);

			aabbs.minVertices[i] = aabb.Min;
			aabbs.maxVertices[i] = aabb.Max;
		}

		m_pComputeDepthRemapTexSP->bind();

		m_pComputeDepthRemapTexSP->updateUniform1i("uAABBNum", m_TransparentModels.size());
		glUniform3fv(glGetUniformLocation(m_pComputeDepthRemapTexSP->getProgramID(), "minAABBVertices"), MAX_NUM_AABB, (const GLfloat*)aabbs.minVertices);
		glUniform3fv(glGetUniformLocation(m_pComputeDepthRemapTexSP->getProgramID(), "maxAABBVertices"), MAX_NUM_AABB, (const GLfloat*)aabbs.maxVertices);

		CRenderer::getInstance()->drawScreenQuad(*m_pComputeDepthRemapTexSP);

		m_pComputeDepthRemapTexSP->unbind();

		m_pComputeDepthRemapTexFrameBuffer->unbind();
#endif
		m_pRenderer->popEvent();

		m_pRenderer->pushEvent("Projection");
		m_pWOITProjectionFrameBuffer->bind();
		glViewport(0, 0, COEFF_MAP_WIDTH, COEFF_MAP_HEIGHT);
		CRenderer::getInstance()->setClearColor(0, 0, 0, 0);
		CRenderer::getInstance()->clear();
		CRenderer::getInstance()->enableCullFace(false);
		CRenderer::getInstance()->setDepthMask(false);
		CRenderer::getInstance()->enableBlend(true);
		CRenderer::getInstance()->setBlendFunc(GL_ONE, GL_ONE);

		m_pGenWaveletOpacityMapSP->bind();
		//m_pOpaqueDepthTex->bindV(2);
		//m_pGenWaveletOpacityMapSP->updateUniformTexture("uOpaqueDepthTex", m_pOpaqueDepthTex.get());

		m_pDepthRemapTex->bindV(2);
		glGenerateMipmap(GL_TEXTURE_2D);
		m_pGenWaveletOpacityMapSP->updateUniformTexture("uDepthRemapTex", m_pDepthRemapTex.get());

#ifdef ENABLE_PRE_INTEGRAL
		m_pPsiIntegralLutTex->bindV(3);
		m_pGenWaveletOpacityMapSP->updateUniformTexture("uPsiIntegralLutTex", m_pPsiIntegralLutTex.get());
#else
		m_pPsiLutTex->bindV(3);
		m_pGenWaveletOpacityMapSP->updateUniformTexture("uPsiLutTex", m_pPsiLutTex.get());
#endif

		for (auto Model : m_TransparentModels)
		{
			auto Material = m_Model2MaterialMap[Model];
			m_pGenWaveletOpacityMapSP->bind();
			m_pGenWaveletOpacityMapSP->updateUniform1f("uCoverage", Material.coverage);
			CRenderer::getInstance()->draw(*Model, *m_pGenWaveletOpacityMapSP);
		}

		CRenderer::getInstance()->setDepthMask(true);
		CRenderer::getInstance()->enableBlend(false);

		m_pWOITProjectionFrameBuffer->unbind();
		m_pRenderer->popEvent();

		m_pRenderer->pushEvent("Reconstruction");
		m_pWOITReconstructionFrameBuffer->bind();
		glViewport(0, 0, TRANSPARENT_TEX_WIDTH, TRANSPARENT_TEX_HEIGHT);
		CRenderer::getInstance()->clear();
		CRenderer::getInstance()->enableCullFace(false);
		CRenderer::getInstance()->setDepthMask(false);
		CRenderer::getInstance()->enableBlend(true);
		CRenderer::getInstance()->setBlendFunc(GL_ONE, GL_ONE);

		m_pWOITReconstructTransmittanceSP->bind();
		//m_pOpaqueDepthTex->bindV(2);
		//m_pWOITReconstructTransmittanceSP->updateUniformTexture("uOpaqueDepthTex", m_pOpaqueDepthTex.get());

		m_pDepthRemapTex->bindV(2);
		m_pWOITReconstructTransmittanceSP->updateUniformTexture("uDepthRemapTex", m_pDepthRemapTex.get());

#ifdef ENABLE_PRE_INTEGRAL
		m_pPsiLutTex->bindV(3);
		m_pWOITReconstructTransmittanceSP->updateUniformTexture("uPsiLutTex", m_pPsiLutTex.get());
#else
		m_pPsiIntegralLutTex->bindV(3);
		m_pWOITReconstructTransmittanceSP->updateUniformTexture("uPsiIntegralLutTex", m_pPsiIntegralLutTex.get());
#endif

#ifndef WOIT_ENABLE_QUANTIZATION
		m_pWaveletOpacityMap1->bindV(4);
		m_pWaveletOpacityMap2->bindV(5);
		m_pWaveletOpacityMap3->bindV(6);
		m_pWaveletOpacityMap4->bindV(7);
		m_pWOITReconstructTransmittanceSP->updateUniformTexture("uWaveletCoeffsMap1", m_pWaveletOpacityMap1.get());
		m_pWOITReconstructTransmittanceSP->updateUniformTexture("uWaveletCoeffsMap2", m_pWaveletOpacityMap2.get());
		m_pWOITReconstructTransmittanceSP->updateUniformTexture("uWaveletCoeffsMap3", m_pWaveletOpacityMap3.get());
		m_pWOITReconstructTransmittanceSP->updateUniformTexture("uWaveletCoeffsMap4", m_pWaveletOpacityMap4.get());
#endif

		for (auto Model : m_TransparentModels)
		{
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
			auto Material = m_Model2MaterialMap[Model];
			m_pWOITReconstructTransmittanceSP->bind();
			m_pWOITReconstructTransmittanceSP->updateUniform3f("uDiffuseColor", Material.diffuse);
			m_pWOITReconstructTransmittanceSP->updateUniform1f("uCoverage", Material.coverage);
			CRenderer::getInstance()->draw(*Model, *m_pWOITReconstructTransmittanceSP);
		}

		CRenderer::getInstance()->setDepthMask(true);
		CRenderer::getInstance()->enableBlend(false);

		m_pWOITReconstructionFrameBuffer->unbind();
		m_pRenderer->popEvent();

		m_pRenderer->pushEvent("Merge Color");
		glViewport(0, 0, WIN_WIDTH, WIN_HEIGHT);
		CRenderer::getInstance()->clear();

		m_pOpaqueColorTex->bindV(0);
		m_pTransparencyColorTex->bindV(1);
		m_pTotalAbsorbanceTex->bindV(2);

		m_pWOITMergerColorSP->bind();
		m_pWOITMergerColorSP->updateUniformTexture("uOpaqueColorTex", m_pOpaqueColorTex.get());
		m_pWOITMergerColorSP->updateUniformTexture("uTranslucentColorTex", m_pTransparencyColorTex.get());
		m_pWOITMergerColorSP->updateUniformTexture("uTotalAbsorbanceTex", m_pTotalAbsorbanceTex.get());

		CRenderer::getInstance()->drawScreenQuad(*m_pWOITMergerColorSP);
		m_pRenderer->popEvent();
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

	CScene		m_Scene;
	CRenderer*	m_pRenderer = nullptr;

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
	std::shared_ptr<CTexture2D>		m_pMomentB0Map;
	std::shared_ptr<CTexture2D>		m_pMomentsMap1;
	std::shared_ptr<CTexture2D>		m_pMomentsMap2;
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

#if defined(USING_LINKED_LIST_OIT) || defined(USING_MOMENT_BASED_OIT) || defined(USING_WAVELET_OIT)
	std::unique_ptr<CFrameBuffer> m_pClearImageFrameBuffer;
#endif

#ifdef USING_WAVELET_OIT
	std::unique_ptr<CShaderProgram> m_pGenWaveletOpacityMapSP;
	std::unique_ptr<CShaderProgram> m_pWOITReconstructTransmittanceSP;
	std::unique_ptr<CShaderProgram> m_pWOITMergerColorSP;
	std::unique_ptr<CShaderProgram> m_pComputeDepthRemapTexSP;

	std::unique_ptr<CFrameBuffer>	m_pClearWaveletOpacityMapFrameBuffer;
	std::unique_ptr<CFrameBuffer>	m_pComputeDepthRemapTexFrameBuffer;
	std::unique_ptr<CFrameBuffer>	m_pWOITProjectionFrameBuffer;
	std::unique_ptr<CFrameBuffer>	m_pWOITReconstructionFrameBuffer;

#ifdef WOIT_ENABLE_QUANTIZATION
	std::shared_ptr<CImage2D>	m_pWaveletOpacityMap1;
	std::shared_ptr<CImage2D>	m_pWaveletOpacityMap2;
	std::shared_ptr<CImage2D>	m_pWaveletOpacityMap3;
	std::shared_ptr<CImage2D>	m_pWaveletOpacityMap4;
#else
	std::shared_ptr<CTexture2D>	m_pWaveletOpacityMap1;
	std::shared_ptr<CTexture2D>	m_pWaveletOpacityMap2;
	std::shared_ptr<CTexture2D>	m_pWaveletOpacityMap3;
	std::shared_ptr<CTexture2D>	m_pWaveletOpacityMap4;
#endif

	std::shared_ptr<CTexture2D>		m_pPsiLutTex;
	std::shared_ptr<CTexture2D>		m_pPsiIntegralLutTex;
	std::shared_ptr<CTexture2D>		m_pTotalAbsorbanceTex;
	std::shared_ptr<CTexture2D>		m_pDepthRemapTex;
#endif
};

int main()
{
	CMyApplication App;
	if (!App.init(SWindowInfo(WIN_WIDTH, WIN_HEIGHT, "Order Independent Transparency Demo"))) return -1;
	App.run();

	return 0;
}