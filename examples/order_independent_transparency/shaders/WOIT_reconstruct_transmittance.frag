#version 460 core
#extension GL_NV_shader_atomic_float : require
#extension GL_ARB_shader_image_load_store : require

#include "common.glsl"
#include "compute_phong_shading.glsl"
#include "WOIT_common.glsl"

uniform sampler2D		uMaterialDiffuseTex;
uniform sampler2D		uMaterialSpecularTex;
uniform sampler2D		uOpaqueDepthTex;
uniform sampler2D		uDepthRemapTex;

#ifdef ENABLE_PRE_INTEGRAL
	uniform sampler2D		uPsiLutTex;
#else
	uniform sampler2D		uPsiIntegralLutTex;
#endif

#ifndef WOIT_ENABLE_QUANTIZATION
	uniform sampler2D		uWaveletCoeffsMap1;
	uniform sampler2D		uWaveletCoeffsMap2;
	uniform sampler2D		uWaveletCoeffsMap3;
	uniform sampler2D		uWaveletCoeffsMap4;
#else
	layout(binding = 0, rgba8ui) coherent uniform uimage2D	uWaveletCoeffsMap1;
	layout(binding = 1, rgba8ui) coherent uniform uimage2D	uWaveletCoeffsMap2;
	layout(binding = 2, rgba8ui) coherent uniform uimage2D	uWaveletCoeffsMap3;
	layout(binding = 3, rgba8ui) coherent uniform uimage2D	uWaveletCoeffsMap4;
#endif

uniform vec3	uViewPos = vec3(0.0);
uniform vec3	uDiffuseColor;
uniform float	uCoverage;

layout(location = 0) in vec3 _inPositionW;
layout(location = 1) in vec3 _inNormalW;
layout(location = 2) in vec2 _inTexCoord;

layout(location = 0) out vec4 _outTransparencyColor;

#include "compute_reflection_color.glsl"

float wavelet_basis_integral(float d, int i)
{
	int indexX = int(BASIS_SLICE_COUNT * d);
	indexX = min(max(indexX, 0), BASIS_SLICE_COUNT - 1);
	int indexY = i;

#ifdef ENABLE_PRE_INTEGRAL
	return 2 * BASIS_SCALE * texelFetch(uPsiLutTex, ivec2(indexX, indexY), 0).r - BASIS_SCALE;
#else
	return 2 * BASIS_SCALE * texelFetch(uPsiIntegralLutTex, ivec2(indexX, indexY), 0).r - BASIS_SCALE;
#endif
}

float basisIntegralFunc(float x, int i)
{
	float result = 0;
	int indexJ = (i != 0) ? int(floor(log2(float(i)))) : 0;
	int indexK = int(i - pow(2, indexJ));

#if BASIS_TYPE == FOURIER_BASIS
	if (i == 0) 
	{
		result = x / 2.0;
	}
	else
	{
		int k = (i - 1) / 2 + 1;
		result = (i % 2 == 1) ? sin(2*PI*k*x) : (1 - cos(2*PI*k*x));
		result /= 2 * PI * k;
		result *= SIGMA_K(k, BASIS_NUM / 2);
	}
#elif BASIS_TYPE == HAAR_BASIS
	if (i == 0)
		result = haar_phi_integral(x);
	else
		result = haar_psi_integral(x, indexJ, indexK);
#elif BASIS_TYPE == WAVELET_BASIS
	result = wavelet_basis_integral(x, i);
#endif
	return result;
}

void main()
{
	//float depth = texelFetch(uOpaqueDepthTex, ivec2(gl_FragCoord.xy), 0).r;
	//if (depth != 0.0 && gl_FragCoord.z > depth) discard;

	float depth = _linearizeDepth(gl_FragCoord.z, NEAR_PLANE, FAR_PLANE);
    vec2 texCoord = gl_FragCoord.xy / vec2(WIN_WIDTH, WIN_HEIGHT);

#ifdef ENABLE_DEPTH_REMAPPING
	vec2 minMaxZ = textureLod(uDepthRemapTex, texCoord, 0).xy;
	depth = remap(depth, minMaxZ.x, minMaxZ.y, 0.01, 0.99);
#endif

	float opticalDepth = 0.0;
	int basisIndex = 0;

#ifndef WOIT_ENABLE_QUANTIZATION
	if (BASIS_NUM >= 4)
	{
	#ifdef DISCARD_UNUSED_BASIS_REC
		if (depth >= 0 && depth <= 5.0/16.0)
	#endif
		{
			vec4 coeffs = texture(uWaveletCoeffsMap1, texCoord);
			opticalDepth += basisIntegralFunc(depth, basisIndex) * coeffs.x;
			opticalDepth += basisIntegralFunc(depth, basisIndex + 1) * coeffs.y;
			opticalDepth += basisIntegralFunc(depth, basisIndex + 2) * coeffs.z;
			opticalDepth += basisIntegralFunc(depth, basisIndex + 3) * coeffs.w;
		}
		basisIndex += 4;
	}

	if (BASIS_NUM >= 8)
	{
	#ifdef DISCARD_UNUSED_BASIS_REC
		if (depth >= 3.0/16.0 && depth <= 9.0/16.0)
	#endif
		{
			vec4 coeffs = texture(uWaveletCoeffsMap2, texCoord);
			opticalDepth += basisIntegralFunc(depth, basisIndex) * coeffs.x;
			opticalDepth += basisIntegralFunc(depth, basisIndex + 1) * coeffs.y;
			opticalDepth += basisIntegralFunc(depth, basisIndex + 2) * coeffs.z;
			opticalDepth += basisIntegralFunc(depth, basisIndex + 3) * coeffs.w;
		}
		basisIndex += 4;
	}

	if (BASIS_NUM >= 12)
	{
	#ifdef DISCARD_UNUSED_BASIS_REC
		if (depth >= 7.0/16.0 && depth <= 13.0/16.0)
	#endif
		{
			vec4 coeffs = texture(uWaveletCoeffsMap3, texCoord);
			opticalDepth += basisIntegralFunc(depth, basisIndex) * coeffs.x;
			opticalDepth += basisIntegralFunc(depth, basisIndex + 1) * coeffs.y;
			opticalDepth += basisIntegralFunc(depth, basisIndex + 2) * coeffs.z;
			opticalDepth += basisIntegralFunc(depth, basisIndex + 3) * coeffs.w;
		}
		basisIndex += 4;
	}

	if (BASIS_NUM >= 16)
	{
	#ifdef DISCARD_UNUSED_BASIS_REC
		if (depth >= 11.0/16.0 && depth <= 16.0/16.0)
	#endif
		{
			vec4 coeffs = texture(uWaveletCoeffsMap4, texCoord);
			opticalDepth += basisIntegralFunc(depth, basisIndex) * coeffs.x;
			opticalDepth += basisIntegralFunc(depth, basisIndex + 1) * coeffs.y;
			opticalDepth += basisIntegralFunc(depth, basisIndex + 2) * coeffs.z;
			opticalDepth += basisIntegralFunc(depth, basisIndex + 3) * coeffs.w;
		}
		basisIndex += 4;
	}
#else
	if (BASIS_NUM >= 4)
	{
		vec4 coeffs = dequantize(imageLoad(uWaveletCoeffsMap1, ivec2(gl_FragCoord.xy)));
		coeffs = expandFuncMuReverse(coeffs, _IntervalMin, _IntervalMax, _Mu);

		opticalDepth += basisIntegralFunc(depth, basisIndex++) * coeffs.x;
		opticalDepth += basisIntegralFunc(depth, basisIndex++) * coeffs.y;
		opticalDepth += basisIntegralFunc(depth, basisIndex++) * coeffs.z;
		opticalDepth += basisIntegralFunc(depth, basisIndex++) * coeffs.w;
	}

	if (BASIS_NUM >= 8)
	{
		vec4 coeffs = dequantize(imageLoad(uWaveletCoeffsMap2, ivec2(gl_FragCoord.xy)));
		coeffs = expandFuncMuReverse(coeffs, _IntervalMin, _IntervalMax, _Mu);

		opticalDepth += basisIntegralFunc(depth, basisIndex++) * coeffs.x;
		opticalDepth += basisIntegralFunc(depth, basisIndex++) * coeffs.y;
		opticalDepth += basisIntegralFunc(depth, basisIndex++) * coeffs.z;
		opticalDepth += basisIntegralFunc(depth, basisIndex++) * coeffs.w;
	}

	if (BASIS_NUM >= 12)
	{
		vec4 coeffs = dequantize(imageLoad(uWaveletCoeffsMap3, ivec2(gl_FragCoord.xy)));
		coeffs = expandFuncMuReverse(coeffs, _IntervalMin, _IntervalMax, _Mu);

		opticalDepth += basisIntegralFunc(depth, basisIndex++) * coeffs.x;
		opticalDepth += basisIntegralFunc(depth, basisIndex++) * coeffs.y;
		opticalDepth += basisIntegralFunc(depth, basisIndex++) * coeffs.z;
		opticalDepth += basisIntegralFunc(depth, basisIndex++) * coeffs.w;
	}

	if (BASIS_NUM >= 16)
	{
		vec4 coeffs = dequantize(imageLoad(uWaveletCoeffsMap4, ivec2(gl_FragCoord.xy)));
		coeffs = expandFuncMuReverse(coeffs, _IntervalMin, _IntervalMax, _Mu);

		opticalDepth += basisIntegralFunc(depth, basisIndex++) * coeffs.x;
		opticalDepth += basisIntegralFunc(depth, basisIndex++) * coeffs.y;
		opticalDepth += basisIntegralFunc(depth, basisIndex++) * coeffs.z;
		opticalDepth += basisIntegralFunc(depth, basisIndex++) * coeffs.w;
	}
#endif

	float transmittance = exp(-opticalDepth);
	transmittance = clamp(transmittance, 0, 1);

	vec3 reflectColor = computeReflectColor();
	_outTransparencyColor.rgb = transmittance * uCoverage * reflectColor;
	_outTransparencyColor.a = transmittance * uCoverage;
}