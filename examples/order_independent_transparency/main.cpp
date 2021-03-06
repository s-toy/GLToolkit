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
		setDisplayStatusHint();

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

		//float zOffset = sin(9*getTime());
		//m_TransparentModels[3]->setPosition(glm::vec3(0.0, -1.5, 1.0 + zOffset));
		for (int i = 0; i < m_TransparentModels.size(); ++i)
		{
			SAABB AABB = m_TransparentModels[i]->getAABB();
			//auto Min = AABB.Min;
			//auto Max = AABB.Max;

			//std::cout << i << ": " << Max.x - Min.x << ", " << Max.y - Min.y << ", " << Max.z - Min.z << std::endl;
			//std::cout << i << ": " << (Max.x + Min.x) / 2 << ", " << (Max.y + Min.y) / 2 << ", " << (Max.z + Min.z) / 2 << std::endl;
		}

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

#ifdef USING_WAVELET_OIT
#ifdef USING_ALL_METHODS
		if (m_OITMethod == EOITMethod::WAVELET_OIT)
#endif
		{
			if (KeyStatus[GLFW_KEY_0]) m_WOITStrategy = 0;
			else if (KeyStatus[GLFW_KEY_1]) m_WOITStrategy = 1;
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

		//m_pComputeSurfaceZSP = std::make_unique<CShaderProgram>();
		//m_pComputeSurfaceZSP->addShader("shaders/WOIT_compute_surface_z.compute", EShaderType::COMPUTE_SHADER);
		//m_pComputeSurfaceZSP->addShader("shaders/WOIT_compute_surface_z.vert", EShaderType::VERTEX_SHADER);
		//m_pComputeSurfaceZSP->addShader("shaders/WOIT_compute_surface_z.frag", EShaderType::FRAGMENT_SHADER);

		m_pComputeRepresentativeLevelsSP = std::make_unique<CShaderProgram>();
		m_pComputeRepresentativeLevelsSP->addShader("shaders/compute_representative_levels.compute", EShaderType::COMPUTE_SHADER);

		m_pComputeRepresentativeBoundariesSP = std::make_unique<CShaderProgram>();
		m_pComputeRepresentativeBoundariesSP->addShader("shaders/compute_representative_boundaries.compute", EShaderType::COMPUTE_SHADER);
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
		pCamera->setMoveSpeed(0.1);

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

		glGenBuffers(1, &m_ClearWaveletCoeffPDFImagePBO);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_ClearWaveletCoeffPDFImagePBO);
		glBufferData(GL_PIXEL_UNPACK_BUFFER, PDF_SLICE_COUNT * PDF_SLICE_COUNT * 32, NULL, GL_STATIC_DRAW);
		dst = (unsigned*)glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
		memset(dst, 0x00, PDF_SLICE_COUNT * PDF_SLICE_COUNT * 32);
		glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

		m_pWaveletOpacityMaps = std::make_shared<CImage2DArray>();
		m_pWaveletOpacityMaps->createEmpty(WIN_WIDTH, WIN_HEIGHT, COEFF_MAP_COUNT, WOIT_FLT_PRECISION, 0);

		m_pQuantizedWaveletOpacityMaps = std::make_shared<CImage2DArray>();
		m_pQuantizedWaveletOpacityMaps->createEmpty(WIN_WIDTH, WIN_HEIGHT, COEFF_MAP_COUNT, GL_R8UI, 1);

		m_pWaveletCoeffPDFImage = std::make_shared<CImage2D>();
		m_pWaveletCoeffPDFImage->createEmpty(PDF_SLICE_COUNT, PDF_SLICE_COUNT, GL_R32UI, 2);

		m_pRepresentativeDataImage = std::make_shared<CImage2D>();
		m_pRepresentativeDataImage->createEmpty(257, 2, GL_R32F, 3);

		m_pNewRepresentativeDataImage = std::make_shared<CImage2D>();
		m_pNewRepresentativeDataImage->createEmpty(257, 2, GL_R32F, 4);

		m_pSurfaceZImage = std::make_shared<CImage2D>();
		m_pSurfaceZImage->createEmpty(WIN_WIDTH, WIN_HEIGHT, GL_RG16F, 5);

		m_pTotalAbsorbanceTex = std::make_shared<CTexture2D>();
		m_pTotalAbsorbanceTex->createEmpty(WIN_WIDTH, WIN_HEIGHT, GL_R16F, GL_CLAMP_TO_BORDER, GL_NEAREST);

		m_pEmptyTex = std::make_shared<CTexture2D>();
		m_pEmptyTex->createEmpty(WIN_WIDTH, WIN_HEIGHT, GL_R16F, GL_CLAMP_TO_BORDER, GL_NEAREST);

		m_pWOITFrameBuffer1 = std::make_unique<CFrameBuffer>(WIN_WIDTH, WIN_HEIGHT, false);
		m_pWOITFrameBuffer1->set(EAttachment::COLOR0, m_pTotalAbsorbanceTex);

		//m_pWOITSurfaceZFrameBuffer = std::make_unique<CFrameBuffer>(WIN_WIDTH, WIN_HEIGHT);
		//m_pWOITSurfaceZFrameBuffer->set(EAttachment::COLOR0, m_pEmptyTex);

		m_pWOITFrameBuffer2 = std::make_unique<CFrameBuffer>(WIN_WIDTH, WIN_HEIGHT);
		m_pWOITFrameBuffer2->set(EAttachment::COLOR0, m_pTransparencyColorTex);

		m_pPsiIntegralLutTex = std::make_shared<CTexture2D>();
		m_pPsiIntegralLutTex->load16("textures/db2_psi_int_n10_j3_s20.png", GL_CLAMP_TO_BORDER, GL_NEAREST);

		m_pPsiLutTex = std::make_shared<CTexture2D>();
		m_pPsiLutTex->load16("textures/db2_psi_n10_j3_s20.png", GL_CLAMP_TO_BORDER, GL_NEAREST);

		GLfloat data[514] = { 0 };

		for (int i = 257; i < 513; i++)
		{
			//data[i] = float(rand()) / RAND_MAX;
			//data[i] = data[i] * (_IntervalMax - _IntervalMin) + _IntervalMin;
			int k = i - 257;
			if (k < 128)
			{
				float l = pow(2, k * log2(_IntervalMax + 1) / 128) - 1;
				float r = pow(2, (k + 1) * log2(_IntervalMax + 1) / 128) - 1;
				data[i] = (l + r) / 2;
			}
			else
			{
				float l = pow(2, (k - 128) * log2(-_IntervalMin + 1) / 128) - 1;
				float r = pow(2, (k - 127) * log2(-_IntervalMin + 1) / 128) - 1;
				data[i] = -(l + r) / 2;
			}
		}
		std::sort(data + 257, data + 513);

		data[0] = _IntervalMin;
		data[256] = _IntervalMax;
		for (int i = 1; i <= 255; ++i)
		{
			data[i] = 0.5 * (data[i + 256] + data[i + 257]);
		}

		glBindTexture(GL_TEXTURE_2D, m_pRepresentativeDataImage->getObjectID());
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 257, 2, GL_RED, GL_FLOAT, data);
		glBindTexture(GL_TEXTURE_2D, 0);

		glBindTexture(GL_TEXTURE_2D, m_pNewRepresentativeDataImage->getObjectID());
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 257, 2, GL_RED, GL_FLOAT, data);
		glBindTexture(GL_TEXTURE_2D, 0);
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
		for (int i = 0; i < COEFF_MAP_COUNT; i++)
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

			//GLfloat clearColor = 0;
			//glBindTexture(GL_TEXTURE_2D_ARRAY, m_pWaveletOpacityMaps->getObjectID());
			//glClearTexSubImage(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, WIN_WIDTH, WIN_HEIGHT, 1, GL_RED, GL_FLOAT, &clearColor);
			//glBindTexture(GL_TEXTURE_2D_ARRAY, m_pQuantizedWaveletOpacityMaps->getObjectID());
			//glClearTexSubImage(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, WIN_WIDTH, WIN_HEIGHT, 1, GL_RED_INTEGER, GL_UNSIGNED_BYTE, &clearColor);
			//glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
		}

		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_ClearWaveletCoeffPDFImagePBO);
		glBindTexture(GL_TEXTURE_2D, m_pWaveletCoeffPDFImage->getObjectID());
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, PDF_SLICE_COUNT, PDF_SLICE_COUNT, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);

		GLfloat data[514] = { 0 };

		for (int i = 257; i < 513; i++)
		{
			//data[i] = float(rand()) / RAND_MAX;
			//data[i] = data[i] * (_IntervalMax - _IntervalMin) + _IntervalMin;
			int k = i - 257;
			if (k < 128)
			{
				float l = pow(2, k * log2(_IntervalMax + 1) / 128) - 1;
				float r = pow(2, (k + 1) * log2(_IntervalMax + 1) / 128) - 1;
				data[i] = (l + r) / 2;
			}
			else
			{
				float l = pow(2, (k - 128) * log2(-_IntervalMin + 1) / 128) - 1;
				float r = pow(2, (k - 127) * log2(-_IntervalMin + 1) / 128) - 1;
				data[i] = -(l + r) / 2;
			}
		}
		std::sort(data + 257, data + 513);

		data[0] = _IntervalMin;
		data[256] = _IntervalMax;
		for (int i = 1; i <= 255; ++i)
		{
			data[i] = 0.5 * (data[i + 256] + data[i + 257]);
		}

		glBindTexture(GL_TEXTURE_2D, m_pNewRepresentativeDataImage->getObjectID());
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 257, 2, GL_RED, GL_FLOAT, data);
		glBindTexture(GL_TEXTURE_2D, 0);

		m_pClearImageFrameBuffer->bind();
		m_pClearImageFrameBuffer->set(EAttachment::COLOR0, m_pSurfaceZImage);
		CRenderer::getInstance()->setClearColor(10000, -10000, 0, 0);
		CRenderer::getInstance()->clear();
		CRenderer::getInstance()->setClearColor(0, 0, 0, 0);
		m_pClearImageFrameBuffer->unbind();
	}

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

		__clearImages();

		auto pCamera = CRenderer::getInstance()->fetchCamera();

		//pass0: compute surface z
		/*m_pWOITSurfaceZFrameBuffer->bind();

		CRenderer::getInstance()->clear();
		CRenderer::getInstance()->enableCullFace(false);
		CRenderer::getInstance()->setDepthMask(false);

		m_pComputeSurfaceZSP->bind();
		m_pOpaqueDepthTex->bindV(2);
		m_pComputeSurfaceZSP->updateUniformTexture("uOpaqueDepthTex", m_pOpaqueDepthTex.get());
		m_pComputeSurfaceZSP->updateUniform1f("uNearPlane", pCamera->getNear());
		m_pComputeSurfaceZSP->updateUniform1f("uFarPlane", pCamera->getFar());

		for (auto Model : m_TransparentModels)
		{
			auto Material = m_Model2MaterialMap[Model];
			m_pComputeSurfaceZSP->bind();
			CRenderer::getInstance()->draw(*Model, *m_pComputeSurfaceZSP);
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		}

		CRenderer::getInstance()->setDepthMask(true);
		CRenderer::getInstance()->enableBlend(false);

		m_pWOITSurfaceZFrameBuffer->unbind();*/

		//m_pComputeSurfaceZSP->bind();

		//glDispatchCompute((WIN_WIDTH - 1) / 32 + 1, (WIN_HEIGHT - 1) / 32 + 1, 1);

		//m_pComputeSurfaceZSP->unbind();

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
		m_pPsiLutTex->bindV(4);
		m_pGenWaveletOpacityMapSP->updateUniformTexture("uPsiLutTex", m_pPsiLutTex.get());

		m_pGenWaveletOpacityMapSP->updateUniform1f("uNearPlane", pCamera->getNear());
		m_pGenWaveletOpacityMapSP->updateUniform1f("uFarPlane", pCamera->getFar());

		m_pGenWaveletOpacityMapSP->updateUniform1i("uWOITCoeffNum", WOITCoeffNum);

		float representativeDataBuffer[514] = { 0 };
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, m_pRepresentativeDataImage->getObjectID());
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, representativeDataBuffer);
		glBindTexture(GL_TEXTURE_2D, 0);
		m_pGenWaveletOpacityMapSP->updateUniform1fv("uRepresentativeData", 514, representativeDataBuffer);

		for (auto Model : m_TransparentModels)
		{
			auto Material = m_Model2MaterialMap[Model];
			m_pGenWaveletOpacityMapSP->bind();
			m_pGenWaveletOpacityMapSP->updateUniform1f("uCoverage", Material.coverage);
			CRenderer::getInstance()->draw(*Model, *m_pGenWaveletOpacityMapSP);
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		}

		CRenderer::getInstance()->setDepthMask(true);
		CRenderer::getInstance()->enableBlend(false);

		m_pWOITFrameBuffer1->unbind();

		//pass1.2: compute representative levels and boundaries
		for (int i = 0; i < 10; ++i)
		{
			m_pComputeRepresentativeBoundariesSP->bind();
			glDispatchCompute(1, 1, 1);
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
			m_pComputeRepresentativeBoundariesSP->unbind();

			m_pComputeRepresentativeLevelsSP->bind();
			glDispatchCompute(1, 1, 1);
			glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
			m_pComputeRepresentativeLevelsSP->unbind();
		}

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
		m_pWOITReconstructTransmittanceSP->updateUniform1f("uNearPlane", pCamera->getNear());
		m_pWOITReconstructTransmittanceSP->updateUniform1f("uFarPlane", pCamera->getFar());
		m_pWOITReconstructTransmittanceSP->updateUniform1i("uWOITCoeffNum", WOITCoeffNum);
		m_pWOITReconstructTransmittanceSP->updateUniform1fv("uRepresentativeData", 514, representativeDataBuffer);

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

		//pass4: copy representative data
		float newRepresentativeDataBuffer[514] = { 0 };
		glBindTexture(GL_TEXTURE_2D, m_pNewRepresentativeDataImage->getObjectID());
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, newRepresentativeDataBuffer);
		glBindTexture(GL_TEXTURE_2D, 0);

		glBindTexture(GL_TEXTURE_2D, m_pRepresentativeDataImage->getObjectID());
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 257, 2, GL_RED, GL_FLOAT, newRepresentativeDataBuffer);
		glBindTexture(GL_TEXTURE_2D, 0);
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

#if defined(USING_LINKED_LIST_OIT) || defined(USING_MOMENT_BASED_OIT) || defined(USING_WAVELET_OIT)
	std::unique_ptr<CFrameBuffer> m_pClearImageFrameBuffer;
#endif

#ifdef USING_WAVELET_OIT
	std::unique_ptr<CShaderProgram> m_pGenWaveletOpacityMapSP;
	std::unique_ptr<CShaderProgram> m_pWOITReconstructTransmittanceSP;
	std::unique_ptr<CShaderProgram> m_pWOITMergerColorSP;
	std::unique_ptr<CShaderProgram> m_pComputeRepresentativeLevelsSP;
	std::unique_ptr<CShaderProgram> m_pComputeRepresentativeBoundariesSP;
	std::unique_ptr<CShaderProgram> m_pComputeSurfaceZSP;
	//std::unique_ptr<CFrameBuffer>	m_pWOITSurfaceZFrameBuffer;
	std::unique_ptr<CFrameBuffer>	m_pWOITFrameBuffer1;
	std::unique_ptr<CFrameBuffer>	m_pWOITFrameBuffer2;
	std::shared_ptr<CImage2DArray>	m_pWaveletOpacityMaps;
	std::shared_ptr<CImage2DArray>	m_pQuantizedWaveletOpacityMaps;
	std::shared_ptr<CImage2D>		m_pWaveletCoeffPDFImage;
	std::shared_ptr<CImage2D>		m_pRepresentativeDataImage;
	std::shared_ptr<CImage2D>		m_pNewRepresentativeDataImage;
	std::shared_ptr<CImage2D>		m_pSurfaceZImage;

	std::shared_ptr<CTexture2D>		m_pPsiLutTex;
	std::shared_ptr<CTexture2D>		m_pPsiIntegralLutTex;
	std::shared_ptr<CTexture2D>		m_pTotalAbsorbanceTex;
	std::shared_ptr<CTexture2D>		m_pEmptyTex;

	GLuint m_ClearWaveletOpacityMapPBO;
	GLuint m_ClearQuantizedWaveletOpacityMapPBO;
	GLuint m_ClearWaveletCoeffPDFImagePBO;

	int m_WOITStrategy = 0;

	const int PDF_SLICE_COUNT = 100;
	const int COEFF_MAP_COUNT = 16;
#endif
		};

int main()
{
	CMyApplication App;
	if (!App.init(SWindowInfo(WIN_WIDTH, WIN_HEIGHT, "Order Independent Transparency Demo"))) return -1;
	App.run();

	return 0;
}