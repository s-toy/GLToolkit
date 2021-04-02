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

#ifndef M_PI
#define M_PI 3.14159265358979323f
#endif

#define WOIT_FLT_PRECISION GL_R16F

#define USING_WAVELET_OIT

#ifdef USING_ALL_METHODS
#define USING_MOMENT_BASED_OIT
#define USING_WEIGHTED_BLENDED_OIT
#define USING_LINKED_LIST_OIT
#define USING_WAVELET_OIT
#endif

#define WOIT_ENABLE_QERROR_CALCULATION
//#define WOIT_ENABLE_FULL_PDF

enum class EOITMethod : unsigned char
{
	LINKED_LIST_OIT = 0,
	MOMENT_BASE_OIT,
	WEIGHTED_BLENDED_OIT,
	FOURIER_OIT,
	WAVELET_OIT
};

using namespace glt;

const int WIN_WIDTH = 1024;
const int WIN_HEIGHT = 1024;

const float _IntervalMin = -50;
const float _IntervalMax = 50;

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

		m_pTransparencyBlurShaderProgram = std::make_unique<CShaderProgram>();
		m_pTransparencyBlurShaderProgram->addShader("shaders/draw_screen_coord.vert", EShaderType::VERTEX_SHADER);
		m_pTransparencyBlurShaderProgram->addShader("shaders/transparency_blur.frag", EShaderType::FRAGMENT_SHADER);

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

		m_pComputePDFSP = std::make_unique<CShaderProgram>();
		m_pComputePDFSP->addShader("shaders/WOIT_compute_pdf.vert", EShaderType::VERTEX_SHADER);
		m_pComputePDFSP->addShader("shaders/WOIT_compute_pdf.frag", EShaderType::FRAGMENT_SHADER);

		m_pComputeQuantizerParamsSP = std::make_unique<CShaderProgram>();
		m_pComputeQuantizerParamsSP->addShader("shaders/compute_quantizer_parameters.compute", EShaderType::COMPUTE_SHADER);

		m_pComputeRepresentativeLevelsSP = std::make_unique<CShaderProgram>();
		m_pComputeRepresentativeLevelsSP->addShader("shaders/compute_representative_levels.compute", EShaderType::COMPUTE_SHADER);

		m_pComputeRepresentativeBoundariesSP = std::make_unique<CShaderProgram>();
		m_pComputeRepresentativeBoundariesSP->addShader("shaders/compute_representative_boundaries.compute", EShaderType::COMPUTE_SHADER);

		m_pClearCoeffFeatureImageSP = std::make_unique<CShaderProgram>();
		m_pClearCoeffFeatureImageSP->addShader("shaders/WOIT_clear_coeff_feature_image.compute", EShaderType::COMPUTE_SHADER);

#ifdef WOIT_ENABLE_QERROR_CALCULATION
		m_pCalculateQuantizationErrorSP = std::make_unique<CShaderProgram>();
		m_pCalculateQuantizationErrorSP->addShader("shaders/draw_screen_coord.vert", EShaderType::VERTEX_SHADER);
		m_pCalculateQuantizationErrorSP->addShader("shaders/WOIT_calculate_quantization_error.frag", EShaderType::FRAGMENT_SHADER);
#endif

#endif //USING_WAVELET_OIT
		}

	void __initScene()
	{
		CCPUTimer timer;
		timer.start();

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
		m_Scene.load("bmws.json");
		m_OpaqueModels = m_Scene.getModelGroup("opaqueModels");
		m_TransparentModels = m_Scene.getModelGroup("transparentModels");
		for (auto pModel : m_TransparentModels)
		{
			glm::vec4 Params = pModel->getParameters();
			m_Model2MaterialMap[pModel] = SMaterial(glm::vec3(Params.x, Params.y, Params.z), Params.w);
		}

		auto pCamera = CRenderer::getInstance()->fetchCamera();
		pCamera->setPosition(glm::dvec3(0, 0, 3));
		pCamera->setNearPlane(0.1);
		pCamera->setFarPlane(15.0);
		pCamera->setMoveSpeed(0.05);

		if (m_Scene.hasCameraPos())
			pCamera->setPosition(m_Scene.getCameraPos());
		else
			pCamera->setPosition(glm::dvec3(0, 0, 3));
		

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

		m_pTransparencyBlurColorTex = std::make_shared<CTexture2D>();
		m_pTransparencyBlurColorTex->createEmpty(WIN_WIDTH, WIN_HEIGHT, GL_RGBA16F, GL_CLAMP_TO_BORDER, GL_NEAREST);

		m_pTransparencyBlurFrameBuffer = std::make_unique<CFrameBuffer>(WIN_WIDTH, WIN_HEIGHT, false);
		m_pTransparencyBlurFrameBuffer->set(EAttachment::COLOR0, m_pTransparencyBlurColorTex);

		m_pTransparencyBlurSwapFrameBuffer = std::make_unique<CFrameBuffer>(WIN_WIDTH, WIN_HEIGHT, false);
		m_pTransparencyBlurSwapFrameBuffer->set(EAttachment::COLOR0, m_pTransparencyColorTex);

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

#ifdef USING_WAVELET_OIT
		glGenBuffers(1, &m_ClearWaveletOpacityMapPBO);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_ClearWaveletOpacityMapPBO);
		glBufferData(GL_PIXEL_UNPACK_BUFFER, WIN_WIDTH * WIN_HEIGHT * 16, NULL, GL_STATIC_DRAW);
		unsigned* dst = (unsigned*)glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
		memset(dst, 0x00, WIN_WIDTH * WIN_HEIGHT * 16);
		glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

		glGenBuffers(1, &m_ClearQuantizedWaveletOpacityMapPBO);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_ClearQuantizedWaveletOpacityMapPBO);
		glBufferData(GL_PIXEL_UNPACK_BUFFER, WIN_WIDTH * WIN_HEIGHT * 8, NULL, GL_STATIC_DRAW);
		dst = (unsigned*)glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
		memset(dst, 0x00, WIN_WIDTH * WIN_HEIGHT * 8);
		glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

#ifdef WOIT_ENABLE_FULL_PDF
		glGenBuffers(1, &m_ClearWaveletCoeffPDFImagePBO);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_ClearWaveletCoeffPDFImagePBO);
		glBufferData(GL_PIXEL_UNPACK_BUFFER, k_totalTileCount * k_pdfSliceCount * 32, NULL, GL_STATIC_DRAW);
		dst = (unsigned*)glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
		memset(dst, 0x00, k_totalTileCount * k_pdfSliceCount * 32);
		glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
#endif

		m_pWaveletOpacityMaps = std::make_shared<CImage2DArray>();
		m_pWaveletOpacityMaps->createEmpty(WIN_WIDTH, WIN_HEIGHT, k_coeffMapCount, WOIT_FLT_PRECISION, 0);

		m_pQuantizedWaveletOpacityMaps = std::make_shared<CImage2DArray>();
		m_pQuantizedWaveletOpacityMaps->createEmpty(WIN_WIDTH, WIN_HEIGHT, k_coeffMapCount, GL_R8UI, 1);

#ifdef WOIT_ENABLE_FULL_PDF
		m_pWaveletCoeffPDFImage = std::make_shared<CImage2D>();
		m_pWaveletCoeffPDFImage->createEmpty(k_totalTileCount, k_pdfSliceCount, GL_R32UI, 2);
#else
		m_pWaveletCoeffFeatureImage = std::make_shared<CImage2D>();
		m_pWaveletCoeffFeatureImage->createEmpty(k_totalTileCount, 4, GL_R32UI, 2);
#endif

		m_pDefaultQuantizerParamsImage = std::make_shared<CImage3D>();
		m_pDefaultQuantizerParamsImage->createEmpty(k_tileCountW, k_tileCountH, k_tileCountD, GL_RGBA16F, 3, GL_CLAMP_TO_EDGE, GL_NEAREST);

		m_pOptimalQuantizerParamsImage = std::make_shared<CImage2DArray>();
		m_pOptimalQuantizerParamsImage->createEmpty(k_tileCountW, k_tileCountH, 512, GL_R16F, 4, GL_CLAMP_TO_EDGE, GL_NEAREST);

		m_pTotalAbsorbanceTex = std::make_shared<CTexture2D>();
		m_pTotalAbsorbanceTex->createEmpty(WIN_WIDTH, WIN_HEIGHT, GL_R16F, GL_CLAMP_TO_BORDER, GL_NEAREST);

		m_pEmptyTex = std::make_shared<CTexture2D>();
		m_pEmptyTex->createEmpty(k_pdfFrameBufferWidth, k_pdfFrameBufferHeight, GL_R16F, GL_CLAMP_TO_BORDER, GL_NEAREST);
		 
		m_pWOITFrameBuffer1 = std::make_unique<CFrameBuffer>(WIN_WIDTH, WIN_HEIGHT, false);
		m_pWOITFrameBuffer1->set(EAttachment::COLOR0, m_pTotalAbsorbanceTex);

		m_pWOITComputePDFFrameBuffer = std::make_unique<CFrameBuffer>(k_pdfFrameBufferWidth, k_pdfFrameBufferHeight);
		m_pWOITComputePDFFrameBuffer->set(EAttachment::COLOR0, m_pEmptyTex);

		m_pWOITFrameBuffer2 = std::make_unique<CFrameBuffer>(WIN_WIDTH, WIN_HEIGHT);
		m_pWOITFrameBuffer2->set(EAttachment::COLOR0, m_pTransparencyColorTex);

		m_pPsiIntegralLutTex = std::make_shared<CTexture2D>();
		m_pPsiIntegralLutTex->load16("textures/db2_psi_int_n8_j3_s20.png", GL_CLAMP_TO_BORDER, GL_NEAREST);
		//m_pPsiIntegralLutTex->load16("textures/db2_psi_int_n16_j4_s20.png", GL_CLAMP_TO_BORDER, GL_NEAREST);
		//m_pPsiIntegralLutTex->load16("textures/sym2_psi_int_n10_j02_s20.png", GL_CLAMP_TO_BORDER, GL_NEAREST);

		m_pPsiLutTex = std::make_shared<CTexture2D>();
		m_pPsiLutTex->load16("textures/db2_psi_n8_j3_s20.png", GL_CLAMP_TO_BORDER, GL_NEAREST);
		//m_pPsiLutTex->load16("textures/db2_psi_n16_j4_s20.png", GL_CLAMP_TO_BORDER, GL_NEAREST);
		//m_pPsiLutTex->load16("textures/sym2_psi_n10_j02_s20.png", GL_CLAMP_TO_BORDER, GL_NEAREST);

#ifdef WOIT_ENABLE_QERROR_CALCULATION
		m_pQuantizationErrorTex = std::make_shared<CTexture2D>();
		m_pQuantizationErrorTex->createEmpty(WIN_WIDTH, WIN_HEIGHT, GL_R32F, GL_CLAMP_TO_BORDER, GL_NEAREST);

		m_pWOITCalculateQErrorFrameBuffer = std::make_unique<CFrameBuffer>(WIN_WIDTH, WIN_HEIGHT);
		m_pWOITCalculateQErrorFrameBuffer->set(EAttachment::COLOR0, m_pQuantizationErrorTex);

		m_pTotalQuantizationErrorImage = std::make_shared<CImage2D>();
		m_pTotalQuantizationErrorImage->createEmpty(1, 2, GL_R32F, 7);
#endif

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

#ifdef USING_WAVELET_OIT
	void __clearImages()
	{
		for (int i = 0; i < k_coeffMapCount; i++)
		{
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_ClearWaveletOpacityMapPBO);
			glBindTexture(GL_TEXTURE_2D_ARRAY, m_pWaveletOpacityMaps->getObjectID());
			glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, WIN_WIDTH, WIN_HEIGHT, 1, GL_RED, GL_FLOAT, NULL);
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
			glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_ClearQuantizedWaveletOpacityMapPBO);
			glBindTexture(GL_TEXTURE_2D_ARRAY, m_pQuantizedWaveletOpacityMaps->getObjectID());
			glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, WIN_WIDTH, WIN_HEIGHT, 1, GL_RED_INTEGER, GL_UNSIGNED_BYTE, NULL);
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
			glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
		}

#ifdef WOIT_ENABLE_FULL_PDF
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_ClearWaveletCoeffPDFImagePBO);
		glBindTexture(GL_TEXTURE_2D, m_pWaveletCoeffPDFImage->getObjectID());
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, k_totalTileCount, k_pdfSliceCount, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
#endif

#ifdef WOIT_ENABLE_QERROR_CALCULATION
		float data[2] = { 0 };
		glBindTexture(GL_TEXTURE_2D, m_pTotalQuantizationErrorImage->getObjectID());
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 1, 2, GL_RED, GL_FLOAT, &data);
		glBindTexture(GL_TEXTURE_2D, 0);
#endif

		//float partition[257] = { 0 };
		//float codebook[256] = { 0 };
		//float delta = (_IntervalMax - _IntervalMin) / 256;

		//for (int i = 0; i < 256; ++i)
		//{
		//	codebook[i] = (i - 127) * delta - 0.5 * delta;
		//}

		//partition[0] = -1e3;
		//partition[256] = 1e3;
		//for (int i = 1; i < 256; ++i)
		//{
		//	partition[i] = 0.5 * (codebook[i] + codebook[i - 1]);
		//}

		//float data[514] = { 0 };
		//for (int i = 0; i <= 256; ++i) data[i] = partition[i];
		//for (int i = 257; i < 513; ++i) data[i] = codebook[i - 257];

		//glBindTexture(GL_TEXTURE_2D, m_pOptimalQuantizerParamsImage->getObjectID());
		//glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 257, 2, GL_RED, GL_FLOAT, data);
		//glBindTexture(GL_TEXTURE_2D, 0);
	}

	void __renderUsingWaveletOIT()
	{
		__clearImages();

		auto pCamera = CRenderer::getInstance()->fetchCamera();

		//pass0.0: clear coeff feature image
#ifndef WOIT_ENABLE_FULL_PDF
		m_pClearCoeffFeatureImageSP->bind();
		m_pClearCoeffFeatureImageSP->updateUniform1i("uTileCountW", k_tileCountW);
		m_pClearCoeffFeatureImageSP->updateUniform1i("uTileCountD", k_tileCountD);
		glDispatchCompute(k_tileCountW, k_tileCountH, k_tileCountD);
		m_pClearCoeffFeatureImageSP->unbind();
#endif

		//pass0.1: compute pdf
		m_pWOITComputePDFFrameBuffer->bind();
		glViewport(0, 0, k_pdfFrameBufferWidth, k_pdfFrameBufferHeight);

		CRenderer::getInstance()->clear();
		CRenderer::getInstance()->enableCullFace(false);
		CRenderer::getInstance()->setDepthMask(false);

		m_pComputePDFSP->bind();
		m_pOpaqueDepthTex->bindV(2);
		m_pComputePDFSP->updateUniformTexture("uOpaqueDepthTex", m_pOpaqueDepthTex.get());
		m_pPsiLutTex->bindV(3);
		m_pComputePDFSP->updateUniformTexture("uPsiLutTex", m_pPsiLutTex.get());
		m_pComputePDFSP->updateUniform1f("uNearPlane", pCamera->getNear());
		m_pComputePDFSP->updateUniform1f("uFarPlane", pCamera->getFar());
		m_pComputePDFSP->updateUniform1i("uTileSize", k_tileSize);
		m_pComputePDFSP->updateUniform1i("uTileCountW", k_tileCountW);
		m_pComputePDFSP->updateUniform1i("uTileCountD", k_tileCountD);

		float representativeDataBuffer[514] = { 0 };

		for (auto Model : m_TransparentModels)
		{
			auto Material = m_Model2MaterialMap[Model];
			m_pComputePDFSP->bind();
			m_pComputePDFSP->updateUniform1f("uCoverage", Material.coverage);
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
			CRenderer::getInstance()->draw(*Model, *m_pComputePDFSP);
		}

		CRenderer::getInstance()->setDepthMask(true);

		m_pWOITComputePDFFrameBuffer->unbind();

		for (int i = 0; i < k_coeffMapCount; i++) // TODO: performance optimization
		{
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_ClearWaveletOpacityMapPBO);
			glBindTexture(GL_TEXTURE_2D_ARRAY, m_pWaveletOpacityMaps->getObjectID());
			glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, WIN_WIDTH, WIN_HEIGHT, 1, GL_RED, GL_FLOAT, NULL);
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
			glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_ClearQuantizedWaveletOpacityMapPBO);
			glBindTexture(GL_TEXTURE_2D_ARRAY, m_pQuantizedWaveletOpacityMaps->getObjectID());
			glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, WIN_WIDTH, WIN_HEIGHT, 1, GL_RED_INTEGER, GL_UNSIGNED_BYTE, NULL);
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
			glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
		}

		//pass0.2: compute quantizer paramters
		m_pComputeQuantizerParamsSP->bind();
		m_pComputeQuantizerParamsSP->updateUniform1i("uTileCountW", k_tileCountW);
		m_pComputeQuantizerParamsSP->updateUniform1i("uTileCountD", k_tileCountD);
		glDispatchCompute(k_tileCountW, k_tileCountH, k_tileCountD);
		m_pComputeQuantizerParamsSP->unbind();

		//pass0.1: compute representative levels and boundaries
		//for (int i = 0; i < 10; ++i)
		//{
		//	m_pComputeRepresentativeBoundariesSP->bind();
		//	glDispatchCompute(1, 1, 1);
		//	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		//	m_pComputeRepresentativeBoundariesSP->unbind();

		//	m_pComputeRepresentativeLevelsSP->bind();
		//	glDispatchCompute(1, 1, 1);
		//	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		//	m_pComputeRepresentativeLevelsSP->unbind();
		//}

		//pass1: generate wavelet opacity map
		m_pWOITFrameBuffer1->bind();
		glViewport(0, 0, WIN_WIDTH, WIN_HEIGHT);

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
		m_pDefaultQuantizerParamsImage->bindV(4);
		m_pGenWaveletOpacityMapSP->updateUniformTexture("uDefaultQuantizerParamsImage", m_pDefaultQuantizerParamsImage.get());
		m_pOptimalQuantizerParamsImage->bindV(5);
		m_pGenWaveletOpacityMapSP->updateUniformTexture("uOptimalQuantizerParamsImage", m_pOptimalQuantizerParamsImage.get());

		m_pGenWaveletOpacityMapSP->updateUniform1f("uNearPlane", pCamera->getNear());
		m_pGenWaveletOpacityMapSP->updateUniform1f("uFarPlane", pCamera->getFar());
		m_pGenWaveletOpacityMapSP->updateUniform1i("uScaleSize", k_scaleSize);
		m_pGenWaveletOpacityMapSP->updateUniform1i("uTileSize", k_tileSize);
		m_pGenWaveletOpacityMapSP->updateUniform1i("uTileCountW", k_tileCountW);
		m_pGenWaveletOpacityMapSP->updateUniform1i("uTileCountH", k_tileCountH);
		m_pGenWaveletOpacityMapSP->updateUniform1i("uTileCountD", k_tileCountD);

		m_pGenWaveletOpacityMapSP->updateUniform1fv("uRepresentativeData", 514, representativeDataBuffer);

		for (auto Model : m_TransparentModels)
		{
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
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
		m_pPsiIntegralLutTex->bindV(3);
		m_pWOITReconstructTransmittanceSP->updateUniformTexture("uOpaqueDepthTex", m_pOpaqueDepthTex.get());
		m_pWOITReconstructTransmittanceSP->updateUniformTexture("uPsiIntegralLutTex", m_pPsiIntegralLutTex.get());

		m_pDefaultQuantizerParamsImage->bindV(4);
		m_pWOITReconstructTransmittanceSP->updateUniformTexture("uDefaultQuantizerParamsImage", m_pDefaultQuantizerParamsImage.get());
		m_pOptimalQuantizerParamsImage->bindV(5);
		m_pWOITReconstructTransmittanceSP->updateUniformTexture("uOptimalQuantizerParamsImage", m_pOptimalQuantizerParamsImage.get());

		m_pWOITReconstructTransmittanceSP->updateUniform1f("uNearPlane", pCamera->getNear());
		m_pWOITReconstructTransmittanceSP->updateUniform1f("uFarPlane", pCamera->getFar());
		m_pWOITReconstructTransmittanceSP->updateUniform1i("uScaleSize", k_scaleSize);
		m_pWOITReconstructTransmittanceSP->updateUniform1i("uTileSize", k_tileSize);
		m_pWOITReconstructTransmittanceSP->updateUniform1i("uTileCountW", k_tileCountW);
		m_pWOITReconstructTransmittanceSP->updateUniform1i("uTileCountH", k_tileCountH);
		m_pWOITReconstructTransmittanceSP->updateUniform1i("uTileCountD", k_tileCountD);
		m_pWOITReconstructTransmittanceSP->updateUniform1fv("uRepresentativeData", 514, representativeDataBuffer);

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

		m_pWOITFrameBuffer2->unbind();

		//pass2.1: transparency texture filtering
		//m_pTransparencyBlurFrameBuffer->bind();
		//CRenderer::getInstance()->clear();
		//m_pTransparencyBlurShaderProgram->bind();
		//m_pTransparencyColorTex->bindV(0);
		//m_pTransparencyBlurShaderProgram->updateUniformTexture("uInputTex", m_pTransparencyColorTex.get());
		//m_pTransparencyBlurShaderProgram->updateUniform2f("uBlurDirection", glm::vec2(1, 0));
		//CRenderer::getInstance()->drawScreenQuad(*m_pTransparencyBlurShaderProgram);
		//m_pTransparencyBlurFrameBuffer->unbind();

		//m_pTransparencyBlurSwapFrameBuffer->bind();
		//CRenderer::getInstance()->clear();
		//m_pTransparencyBlurShaderProgram->bind();
		//m_pTransparencyBlurColorTex->bindV(0);
		//m_pTransparencyBlurShaderProgram->updateUniformTexture("uInputTex", m_pTransparencyBlurColorTex.get());
		//m_pTransparencyBlurShaderProgram->updateUniform2f("uBlurDirection", glm::vec2(0, 1));
		//CRenderer::getInstance()->drawScreenQuad(*m_pTransparencyBlurShaderProgram);
		//m_pTransparencyBlurSwapFrameBuffer->unbind();

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

#ifdef WOIT_ENABLE_QERROR_CALCULATION
		//pass4: calculate quantization error
		m_pWOITCalculateQErrorFrameBuffer->bind();
		CRenderer::getInstance()->clear();

		m_pCalculateQuantizationErrorSP->bind();
		m_pCalculateQuantizationErrorSP->updateUniform1i("uScaleSize", k_scaleSize);
		m_pCalculateQuantizationErrorSP->updateUniform1i("uTileSize", k_tileSize);
		m_pCalculateQuantizationErrorSP->updateUniform1i("uTileCountW", k_tileCountW);
		m_pCalculateQuantizationErrorSP->updateUniform1i("uTileCountH", k_tileCountH);
		m_pCalculateQuantizationErrorSP->updateUniform1i("uTileCountD", k_tileCountD);

		m_pDefaultQuantizerParamsImage->bindV(0);
		m_pCalculateQuantizationErrorSP->updateUniformTexture("uDefaultQuantizerParamsImage", m_pDefaultQuantizerParamsImage.get());
		m_pOptimalQuantizerParamsImage->bindV(1);
		m_pCalculateQuantizationErrorSP->updateUniformTexture("uOptimalQuantizerParamsImage", m_pOptimalQuantizerParamsImage.get());

		CRenderer::getInstance()->drawScreenQuad(*m_pCalculateQuantizationErrorSP);

		m_pWOITCalculateQErrorFrameBuffer->unbind();

		float totalError[2];
		glBindTexture(GL_TEXTURE_2D, m_pTotalQuantizationErrorImage->getObjectID());
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, &totalError);
		glBindTexture(GL_TEXTURE_2D, 0);

		std::cout << "Total Error: " << totalError[0] << std::endl;
		//std::cout << totalError[1] << std::endl;
		std::cout << "SNR:" << 10 * log10(totalError[1] / totalError[0]) << std::endl;
#endif
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
	std::unique_ptr<CShaderProgram> m_pTransparencyBlurShaderProgram;

	std::unique_ptr<CFrameBuffer>	m_pOpaqueFrameBuffer;
	std::unique_ptr<CFrameBuffer>	m_pTransparencyBlurFrameBuffer;
	std::unique_ptr<CFrameBuffer>	m_pTransparencyBlurSwapFrameBuffer;

	std::shared_ptr<CTexture2D>		m_pOpaqueColorTex;
	std::shared_ptr<CTexture2D>		m_pOpaqueDepthTex;
	std::shared_ptr<CTexture2D>		m_pTransparencyColorTex;
	std::shared_ptr<CTexture2D>		m_pTransparencyBlurColorTex;

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

#if defined(USING_LINKED_LIST_OIT) || defined(USING_MOMENT_BASED_OIT) || defined(USING_WAVELET_OIT)
	std::unique_ptr<CFrameBuffer> m_pClearImageFrameBuffer;
#endif

#ifdef USING_WAVELET_OIT
	std::unique_ptr<CShaderProgram> m_pGenWaveletOpacityMapSP;
	std::unique_ptr<CShaderProgram> m_pWOITReconstructTransmittanceSP;
	std::unique_ptr<CShaderProgram> m_pWOITMergerColorSP;
	std::unique_ptr<CShaderProgram> m_pComputeRepresentativeLevelsSP;
	std::unique_ptr<CShaderProgram> m_pComputeRepresentativeBoundariesSP;
	std::unique_ptr<CShaderProgram> m_pComputePDFSP;
	std::unique_ptr<CShaderProgram> m_pComputeQuantizerParamsSP;
	std::unique_ptr<CShaderProgram> m_pClearCoeffFeatureImageSP;

	std::unique_ptr<CFrameBuffer>	m_pWOITComputePDFFrameBuffer;
	std::unique_ptr<CFrameBuffer>	m_pWOITFrameBuffer1;
	std::unique_ptr<CFrameBuffer>	m_pWOITFrameBuffer2;

#ifdef WOIT_ENABLE_QERROR_CALCULATION
	std::unique_ptr<CShaderProgram> m_pCalculateQuantizationErrorSP;
	std::unique_ptr<CFrameBuffer>	m_pWOITCalculateQErrorFrameBuffer;
	std::shared_ptr<CTexture2D>		m_pQuantizationErrorTex;
	std::shared_ptr<CImage2D>		m_pTotalQuantizationErrorImage;
#endif

	std::shared_ptr<CImage2DArray>	m_pWaveletOpacityMaps;
	std::shared_ptr<CImage2DArray>	m_pQuantizedWaveletOpacityMaps;

#ifdef WOIT_ENABLE_FULL_PDF
	std::shared_ptr<CImage2D>		m_pWaveletCoeffPDFImage;
#else
	std::shared_ptr<CImage2D>		m_pWaveletCoeffFeatureImage;
#endif

	std::shared_ptr<CImage3D>		m_pDefaultQuantizerParamsImage;
	std::shared_ptr<CImage2DArray>	m_pOptimalQuantizerParamsImage;

	std::shared_ptr<CTexture2D>		m_pPsiLutTex;
	std::shared_ptr<CTexture2D>		m_pPsiIntegralLutTex;
	std::shared_ptr<CTexture2D>		m_pTotalAbsorbanceTex;
	std::shared_ptr<CTexture2D>		m_pEmptyTex;

	GLuint m_ClearWaveletOpacityMapPBO;
	GLuint m_ClearQuantizedWaveletOpacityMapPBO;

#ifdef WOIT_ENABLE_FULL_PDF
	GLuint m_ClearWaveletCoeffPDFImagePBO;
#endif

	const int k_coeffMapCount = 16;
	const int k_scaleSize = 2;
	const int k_pdfFrameBufferWidth = WIN_WIDTH / k_scaleSize;
	const int k_pdfFrameBufferHeight = WIN_HEIGHT / k_scaleSize;
	const int k_tileSize = 2048;
	const int k_tileCountW = (k_pdfFrameBufferWidth - 1) / k_tileSize + 1;
	const int k_tileCountH = (k_pdfFrameBufferHeight - 1) / k_tileSize + 1;
	const int k_tileCountD = 1;
	const int k_totalTileCount = k_tileCountW * k_tileCountH * k_tileCountD;
	const int k_pdfSliceCount = 2048; // 需要与shader中的数值保持一致
	const int k_coeffFeatureSize = 4;

#endif
};

int main()
{
	CMyApplication App;
	if (!App.init(SWindowInfo(WIN_WIDTH, WIN_HEIGHT, "Order Independent Transparency Demo"))) return -1;
	App.run();

	return 0;
}