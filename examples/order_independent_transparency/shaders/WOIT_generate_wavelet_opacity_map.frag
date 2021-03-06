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
uniform int			uWOITCoeffNum;

layout(location = 0) in float _inFragDepth;

layout(location = 0) out float _outTotalAbsorbance;

layout(binding = 0, WOIT_FLT_PRECISION) uniform image2DArray uWaveletOpacityMaps;
layout(binding = 1, r8ui) uniform uimage2DArray	uQuantizedWaveletOpacityMaps;
layout(binding = 2, r32ui)	 uniform uimage2D		uWaveletCoeffPDFImage;
layout(binding = 5, rg16f)	 uniform image2D		uSurfaceZImage;

void writePDF(float val)
{
#if defined(WOIT_ENABLE_QUANTIZATION) && (QUANTIZATION_METHOD == LLOYD_MAX_QUANTIZATION)
	int sliceIndex = int(floor(float(PDF_SLICE_COUNT * PDF_SLICE_COUNT) * (val - _IntervalMin) / (_IntervalMax - _IntervalMin)));
	sliceIndex = clamp(sliceIndex, 0, PDF_SLICE_COUNT * PDF_SLICE_COUNT - 1);
	ivec2 coord = ivec2(sliceIndex / PDF_SLICE_COUNT, sliceIndex % PDF_SLICE_COUNT);
	imageAtomicAdd(uWaveletCoeffPDFImage, coord, 1);
#endif
}

void writePDF(vec4 val)
{
#if defined(WOIT_ENABLE_QUANTIZATION) && (QUANTIZATION_METHOD == LLOYD_MAX_QUANTIZATION)
	writePDF(val.x);
	writePDF(val.y);
	writePDF(val.z);
	writePDF(val.w);
#endif
}

void writePDF(vec3 val)
{
#if defined(WOIT_ENABLE_QUANTIZATION) && (QUANTIZATION_METHOD == LLOYD_MAX_QUANTIZATION)
	writePDF(val.x);
	writePDF(val.y);
	writePDF(val.z);
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
	#ifdef USING_DIRECT_PROJECTION
		if (i == 0)
			result = haar_phi_integral(1) - haar_phi_integral(x);
		else
			result = haar_psi_integral(1, indexJ, indexK) - haar_psi_integral(x, indexJ, indexK);
	#else
		if (i == 0)
			result = haar_phi(x);
		else
			result = haar_psi(x, indexJ, indexK);
	#endif

#elif BASIS_TYPE == MEYER_BASIS
	result = meyer_basis(x, i);
#endif

	return result;
}

void main()
{
	float opaqueDepth = texelFetch(uOpaqueDepthTex, ivec2(gl_FragCoord.xy), 0).r;
	if (opaqueDepth != 0.0 && gl_FragCoord.z > opaqueDepth) discard;

	beginInvocationInterlockNV();

	float depth = _linearizeDepth(gl_FragCoord.z, uNearPlane, uFarPlane);
#ifdef ENABLE_DEPTH_REMAPPING
	float nearestSurfaceZ = imageLoad(uSurfaceZImage, ivec2(gl_FragCoord.xy)).x;
	float farthestSurfaceZ = imageLoad(uSurfaceZImage, ivec2(gl_FragCoord.xy)).y;
	depth = remap(depth, nearestSurfaceZ, farthestSurfaceZ, 0.1, 0.9);
#endif

	float absorbance = -log(1.0 - uCoverage + 1e-5);

	_outTotalAbsorbance = absorbance;

	float coeffsIncr[BASIS_NUM];
	for (int i = 0; i < BASIS_NUM; ++i)
	{
		coeffsIncr[i] = basisFunc(depth, i) * absorbance;
	}

	for (int i = 0; i < BASIS_NUM; ++i)
	{
		if (coeffsIncr[i] < 0.001) continue;

#ifndef WOIT_ENABLE_QUANTIZATION
		float coeff = imageLoad(uWaveletOpacityMaps, ivec3(gl_FragCoord.xy, i)).r;
#else
		float coeff = dequantize(imageLoad(uQuantizedWaveletOpacityMaps, ivec3(gl_FragCoord.xy, i)).r);
#endif
		coeff += coeffsIncr[i];
#ifndef WOIT_ENABLE_QUANTIZATION
		imageStore(uWaveletOpacityMaps, ivec3(gl_FragCoord.xy, i), vec4(coeff, 0, 0, 0));
#else
		//writePDF(coeff);
		imageStore(uQuantizedWaveletOpacityMaps, ivec3(gl_FragCoord.xy, i), ivec4(quantize(coeff), 0, 0, 0));
#endif
	}

	endInvocationInterlockNV();
}