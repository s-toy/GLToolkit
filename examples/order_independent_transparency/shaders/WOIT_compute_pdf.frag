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

layout(location = 0) in float _inFragDepth;

layout(location = 0) out float _outEmpty;

layout(binding = 0, WOIT_FLT_PRECISION) uniform image2DArray	uWaveletOpacityMaps;
layout(binding = 1, r8ui)				uniform uimage2DArray	uQuantizedWaveletOpacityMaps;
layout(binding = 2, r32ui)				uniform uimage2D		uPDFImage;

float m_IntervalMin = -50;
float m_IntervalMax = 50;

void writePDF(float val, int tileIndex)
{
	int sliceIndex = int(floor(float(PDF_SLICE_COUNT) * (val - m_IntervalMin) / (m_IntervalMax - m_IntervalMin)));
	sliceIndex = clamp(sliceIndex, 0, PDF_SLICE_COUNT - 1);
	ivec2 coord = ivec2(tileIndex, sliceIndex);
	imageAtomicAdd(uPDFImage, coord, 1);
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

	ivec2 tileCoord = ivec2(gl_FragCoord.xy) / uTileSize;
	int tileIndex = tileCoord.y * uTileCountW + tileCoord.x;

	_outEmpty = 1;

	beginInvocationInterlockNV();

	for (int i = 0; i < BASIS_NUM; ++i)
	{
		if (abs(coeffsIncr[i]) < 0.001) continue;

//#ifndef WOIT_ENABLE_QUANTIZATION
		float coeff = imageLoad(uWaveletOpacityMaps, ivec3(gl_FragCoord.xy, i)).r;
//#else
//		float coeff = dequantize(imageLoad(uQuantizedWaveletOpacityMaps, ivec3(gl_FragCoord.xy, i)).r);
//#endif
		coeff += coeffsIncr[i];
		writePDF(coeff, tileIndex);
//#ifndef WOIT_ENABLE_QUANTIZATION
		imageStore(uWaveletOpacityMaps, ivec3(gl_FragCoord.xy, i), vec4(coeff, 0, 0, 0));
//#else
//		imageStore(uQuantizedWaveletOpacityMaps, ivec3(gl_FragCoord.xy, i), ivec4(quantize(coeff), 0, 0, 0));
//#endif
	}

	endInvocationInterlockNV();
}