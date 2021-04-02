#version 460 core
#extension GL_NV_fragment_shader_interlock : require
#extension GL_NV_shader_atomic_float : require

#include "common.glsl"
#include "WOIT_common.glsl"

uniform sampler2D   uOpaqueDepthTex;
uniform sampler2D	uPsiLutTex;
uniform float		uCoverage;
uniform float		uNearPlane;
uniform float		uFarPlane;
uniform int			uTileSize;
uniform int			uTileCountW;
uniform int			uTileCountD;

layout(location = 0) in float _inFragDepth;

layout(location = 0) out float _outEmpty;

layout(binding = 0, WOIT_FLT_PRECISION) coherent uniform image2DArray	uWaveletOpacityMaps;
layout(binding = 1, r8ui)				coherent uniform uimage2DArray	uQuantizedWaveletOpacityMaps;

#ifdef WOIT_ENABLE_FULL_PDF
	layout(binding = 2, r32ui)				coherent uniform uimage2D		uPDFImage;
#else
	layout(binding = 2, r32ui)				coherent uniform uimage2D		uCoeffFeatureImage;
#endif

void writePDF(float val, int tileIndex)
{
#ifdef WOIT_ENABLE_FULL_PDF
	int sliceIndex = int(floor(float(PDF_SLICE_COUNT) * (val - _IntervalMin) / (_IntervalMax - _IntervalMin)));
	sliceIndex = clamp(sliceIndex, 0, PDF_SLICE_COUNT - 1);
	ivec2 coord = ivec2(tileIndex, sliceIndex);
	imageAtomicAdd(uPDFImage, coord, 1);
#else
	int sliceIndex = int(floor(1e6 * (val - _IntervalMin) / (_IntervalMax - _IntervalMin)));
	sliceIndex = max(sliceIndex, 0);
	imageAtomicMin(uCoeffFeatureImage, ivec2(tileIndex, 0), uint(sliceIndex));
	imageAtomicMax(uCoeffFeatureImage, ivec2(tileIndex, 1), uint(sliceIndex));

	//float minVal = imageLoad(uCoeffFeatureImage, ivec2(tileIndex, 0)).x;
	//imageStore(uCoeffFeatureImage, ivec2(tileIndex, 0), vec4(min(val, minVal), 0, 0, 0));

	//float maxVal = imageLoad(uCoeffFeatureImage, ivec2(tileIndex, 1)).x;
	//imageStore(uCoeffFeatureImage, ivec2(tileIndex, 1), vec4(max(val, maxVal), 0, 0, 0));
#endif
}

float meyer_basis(float d, int i)
{
	int indexX = int(BASIS_SLICE_COUNT * d);
	indexX = min(max(indexX, 0), BASIS_SLICE_COUNT - 1);
	int indexY = i;
	return 2 * BASIS_SCALE * texelFetch(uPsiLutTex, ivec2(indexX, indexY), 0).r - BASIS_SCALE;
}

float basisFunc(float x, int i)
{
	float result = 0;
	int indexJ = (i != 0) ? int(floor(log2(float(i)))) : 0;
	int indexK = int(i - pow(2, indexJ));

#if BASIS_TYPE == FOURIER_BASIS
	if (i == 0) 
	{
		result = 2;
	}
	else
	{
		int k = (i - 1) / 2 + 1;
		result = (i % 2 == 1) ? 2*cos(2*PI*k*x) : 2*sin(2*PI*k*x);
	}
#elif BASIS_TYPE == HAAR_BASIS
	if (i == 0)
		result = haar_phi(x);
	else
		result = haar_psi(x, indexJ, indexK);
#elif BASIS_TYPE == MEYER_BASIS
	result = meyer_basis(x, i);
#endif

	return result;
}

void main()
{
	_outEmpty = 1;

	float opaqueDepth = texelFetch(uOpaqueDepthTex, ivec2(gl_FragCoord.xy), 0).r; //TODO
	if (opaqueDepth != 0.0 && gl_FragCoord.z > opaqueDepth) discard;

	float depth = _linearizeDepth(gl_FragCoord.z, uNearPlane, uFarPlane);
	float absorbance = -log(1.0 - uCoverage + 1e-5);

	float coeffsIncr[BASIS_NUM];
	for (int i = 0; i < BASIS_NUM; ++i)
	{
		float basisValue = basisFunc(depth, i);
		coeffsIncr[i] = basisValue * absorbance;
		//if (abs(basisValue) > 0.001) writePDF2(basisValue);
	}

	ivec3 tileCoord;
	tileCoord.xy = ivec2(gl_FragCoord.xy) / uTileSize;
	int basisNumPerTile = (BASIS_NUM - 1) / uTileCountD + 1;

	beginInvocationInterlockNV();

	for (int i = 0; i < BASIS_NUM; ++i)
	{
		//if (abs(coeffsIncr[i]) < 0.001) continue;

		tileCoord.z = i / basisNumPerTile;
		//tileCoord.z = int(floor(depth * float(uTileCountD)));
		//tileCoord.z = clamp(tileCoord.z, 0, uTileCountD - 1);
		int tileIndex = (tileCoord.y * uTileCountW + tileCoord.x) * uTileCountD + tileCoord.z;

		float coeff = imageLoad(uWaveletOpacityMaps, ivec3(gl_FragCoord.xy, i)).r;
		coeff += coeffsIncr[i];
	#if QUANTIZATION_METHOD == UNIFORM_QUANTIZATION
		writePDF(expandFuncMiu(coeff, _IntervalMin, _IntervalMax, _Mu), tileIndex);
	#elif QUANTIZATION_METHOD == LLOYD_MAX_QUANTIZATION
		writePDF(coeff, tileIndex); 

	#endif
		imageStore(uWaveletOpacityMaps, ivec3(gl_FragCoord.xy, i), vec4(coeff, 0, 0, 0));
	}

	endInvocationInterlockNV();
}