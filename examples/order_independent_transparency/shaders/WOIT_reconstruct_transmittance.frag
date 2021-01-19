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
	return 20 * texelFetch(uPsiIntegralLutTex, ivec2(indexX, BASIS_NUM - 1), 0).r - 10;
}

float meyer_psi_integral(float d, float j, float k)
{
	int indexX = int(SLICE_COUNT * d);
	indexX = min(max(indexX, 0), SLICE_COUNT - 1);
	int indexY = int(pow(2, j) - 1.0 + k);
	return 20 * texelFetch(uPsiIntegralLutTex, ivec2(indexX, indexY), 0).r - 10;
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
	}
#elif BASIS_TYPE == HAAR_BASIS
	if (i == 0)
		result = haar_phi_integral(x);
	else
		result = haar_psi_integral(x, indexJ, indexK);
#elif BASIS_TYPE == MEYER_BASIS
	if (i == 0)
		result = meyer_phi_integral(x);
	else
		result = meyer_psi_integral(x, indexJ, indexK);
#elif BASIS_TYPE == SIN_BASIS
	float n = BASIS_NUM;
	float l = 1.0 / n;

	if ((x >= i * l))
	{
		x = min(x, (i + 1) * l);
		//result = (1 - cos(n * PI * (x - i * l))) * sqrt(2 * n) / (n * PI);
		result = 2.3094 * ( (-cos(2*n*PI*(x-i*l)-PI/2)) / (2*n*PI) + x );
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