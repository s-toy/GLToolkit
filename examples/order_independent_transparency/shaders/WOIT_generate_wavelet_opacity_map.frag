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
layout(binding = 1, rgba8ui) uniform uimage2DArray	uQuantizedWaveletOpacityMaps;
layout(binding = 2, r32ui)	 uniform uimage2D		uWaveletCoeffPDFImage;

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

float haar_phi(float x)
{
	return 1;
}

float haar_psi(float x, float j, float k)
{
	float value = pow(2.0f, j / 2.0f);
	//value *= value;

	const float intervalLength = 1.0f / pow(2.0f, j + 1);
	const float intervalMin = 2 * k * intervalLength;
	const float intervalMid = intervalMin + intervalLength;
	const float intervalMax = intervalMid + intervalLength;

	if (x >= intervalMin && x < intervalMid)
		return value;
	else if (x >= intervalMid && x < intervalMax)
		return -value;
	else
		return 0;
}

float meyer_phi(float d)
{
	int indexX = int(SLICE_COUNT * d);
	indexX = min(max(indexX, 0), SLICE_COUNT - 1);
	return 20 * texelFetch(uPsiLutTex, ivec2(indexX, BASIS_NUM - 1), 0).r - 10;
}

float meyer_psi(float d, float j, float k)
{
	int indexX = int(SLICE_COUNT * d);
	indexX = min(max(indexX, 0), SLICE_COUNT - 1);
	int indexY = int(pow(2, j) - 1.0 + k);
	return 20 * texelFetch(uPsiLutTex, ivec2(indexX, indexY), 0).r - 10;
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
	if (i == 0)
		result = meyer_phi(x);
	else
		result = meyer_psi(x, indexJ, indexK);
#elif BASIS_TYPE == SIN_BASIS
	float n = BASIS_NUM;
	float l = 1.0 / n;

	if ((x >= i * l) && (x < (i + 1) * l))
	{
		result = sin(n * PI * (x - i * l)) * sqrt(2 * n);
		//result = 2.3094 * (sin(2*n*PI*(x-i*l)-PI/2) + 1);
	}
	else
	{
		result = 0;
	}
#endif

	return result;
}

void main()
{
	float opaqueDepth = texelFetch(uOpaqueDepthTex, ivec2(gl_FragCoord.xy), 0).r;
	if (opaqueDepth != 0.0 && gl_FragCoord.z > opaqueDepth) discard;

	float depth = _linearizeDepth(gl_FragCoord.z, uNearPlane, uFarPlane);
	float absorbance = -log(1.0 - uCoverage + 1e-5);

	_outTotalAbsorbance = absorbance;

	beginInvocationInterlockNV();

	float coeffsIncr[BASIS_NUM];
	for (int i = 0; i < BASIS_NUM; ++i)
	{
		coeffsIncr[i] = basisFunc(depth, i) * absorbance;
	}

	int loopCount = int(ceil(float(BASIS_NUM) / 4.0));
	for (int i = 0; i < loopCount; ++i)
	{
#ifndef WOIT_ENABLE_QUANTIZATION
		vec4 coeffs = imageLoad(uWaveletOpacityMaps, ivec3(gl_FragCoord.xy, i));
#else
		vec4 coeffs = dequantizeVec4(imageLoad(uQuantizedWaveletOpacityMaps, ivec3(gl_FragCoord.xy, i)));
#endif
		coeffs.x += coeffsIncr[4 * i];
		if (4 * i + 1 < BASIS_NUM) coeffs.y += coeffsIncr[4 * i + 1];
		if (4 * i + 2 < BASIS_NUM) coeffs.z += coeffsIncr[4 * i + 2];
		if (4 * i + 3 < BASIS_NUM) coeffs.w += coeffsIncr[4 * i + 3];
#ifndef WOIT_ENABLE_QUANTIZATION
		imageStore(uWaveletOpacityMaps, ivec3(gl_FragCoord.xy, i), coeffs);
#else
		writePDF(coeffs);
		imageStore(uQuantizedWaveletOpacityMaps, ivec3(gl_FragCoord.xy, i), quantizeVec4(coeffs));
#endif
	}

	endInvocationInterlockNV();
}