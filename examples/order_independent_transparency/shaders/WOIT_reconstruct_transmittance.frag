#version 460 core
#include "common.glsl"
#include "WOIT_common.glsl"
#include "compute_phong_shading.glsl"

layout(binding = 0, WOIT_FLT_PRECISION) uniform image2DArray uWaveletOpacityMaps;
layout(binding = 1, rgba8ui) uniform uimage2DArray uQuantizedWaveletOpacityMaps;

uniform sampler2D	uMaterialDiffuseTex;
uniform sampler2D	uMaterialSpecularTex;
uniform sampler2D   uOpaqueDepthTex;
uniform sampler2D	uPsiIntegralLutTex;

uniform vec3	uViewPos = vec3(0.0);
uniform vec3	uDiffuseColor;
uniform float	uCoverage;
uniform float	uNearPlane;
uniform float	uFarPlane;
uniform int		uWOITCoeffNum;

layout(location = 0) in vec3 _inPositionW;
layout(location = 1) in vec3 _inNormalW;
layout(location = 2) in vec2 _inTexCoord;

layout(location = 0) out vec4 _outTransparencyColor;

#include "compute_reflection_color.glsl"

float haar_phi_integral(float d)
{
	return d;
}

float haar_psi_integral(float d, float j, float k)
{
	const float value = pow(2.0f, j / 2.0f);
	const float intervalLength = 1.0f / pow(2.0f, j + 1);
	const float intervalMin = 2 * k * intervalLength;
	const float intervalMid = intervalMin + intervalLength;
	const float intervalMax = intervalMid + intervalLength;

	if (d < intervalMin)
		return 0;
	else if (d >= intervalMin && d < intervalMid)
		return value * (d - intervalMin);
	else if (d >= intervalMid && d < intervalMax)
		return value * (intervalMid - intervalMin) + (-value) * (d - intervalMid);
	else
		return 0;
}

float meyer_phi_integral(float d)
{
	int indexX = int(SLICE_COUNT * d);
	indexX = min(max(indexX, 0), SLICE_COUNT - 1);
	return texelFetch(uPsiIntegralLutTex, ivec2(indexX, BASIS_NUM - 1), 0).r;
}

float meyer_psi_integral(float d, float j, float k)
{
	int indexX = int(SLICE_COUNT * d);
	indexX = min(max(indexX, 0), SLICE_COUNT - 1);
	int indexY = int(pow(2, j) - 1.0 + k);
	return texelFetch(uPsiIntegralLutTex, ivec2(indexX, indexY), 0).r;
}

float phi_integral(float d)
{
#ifdef USING_HAAR_WAVELETS
	return haar_phi_integral(d);
#else
	return meyer_phi_integral(d) * 20 - 10;
#endif
}

float psi_integral(float d, float j, float k)
{
#ifdef USING_HAAR_WAVELETS
	return haar_psi_integral(d, j, k);
#else
	return meyer_psi_integral(d, j, k) * 20 - 10;
#endif
}

float basisIntegralFunc(float x, int i)
{
	float y = 0;
#if BASIS_TYPE == FOURIER_BASIS
	if (i == 0) 
	{
		y = x / 2.0;
	}
	else
	{
		int k = (i - 1) / 2 + 1;
		y = (i % 2 == 1) ? sin(2*PI*k*x) : (1 - cos(2*PI*k*x));
		y /= 2 * PI * k;
	}
#elif 

#endif
	return y;
}

void main()
{
	float depth = texelFetch(uOpaqueDepthTex, ivec2(gl_FragCoord.xy), 0).r;
	if (depth != 0.0 && gl_FragCoord.z > depth) discard;

	depth = _linearizeDepth(gl_FragCoord.z, uNearPlane, uFarPlane);

	float opticalDepth = 0.0;

	float basisIntegral[BASIS_NUM];
	for (int i = 0; i < BASIS_NUM; ++i)
	{
		basisIntegral[i] = basisIntegralFunc(depth, i);
	}

	int loopCount = int(ceil(float(BASIS_NUM) / 4.0));
	for (int i = 0; i < loopCount; ++i)
	{
#ifndef WOIT_ENABLE_QUANTIZATION
		vec4 coeffs = imageLoad(uWaveletOpacityMaps, ivec3(gl_FragCoord.xy, i));
#else
		vec4 coeffs = dequantizeVec4(imageLoad(uQuantizedWaveletOpacityMaps, ivec3(gl_FragCoord.xy, i)));
#endif
		opticalDepth += coeffs.x * basisIntegral[4 * i];
		if (4 * i + 1 < BASIS_NUM) opticalDepth += coeffs.y * basisIntegral[4 * i + 1];
		if (4 * i + 2 < BASIS_NUM) opticalDepth += coeffs.z * basisIntegral[4 * i + 2];
		if (4 * i + 3 < BASIS_NUM) opticalDepth += coeffs.w * basisIntegral[4 * i + 3];
	}

	float transmittance = exp(-opticalDepth);

	vec3 reflectColor = computeReflectColor();
	_outTransparencyColor.rgb = transmittance * uCoverage * reflectColor;
	_outTransparencyColor.a = transmittance * uCoverage;
}