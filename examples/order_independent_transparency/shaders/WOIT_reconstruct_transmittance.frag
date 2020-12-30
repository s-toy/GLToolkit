#version 460 core
#include "common.glsl"
#include "WOIT_common.glsl"
#include "compute_phong_shading.glsl"

#define PI 3.1415926
#define BASIS_NUM 8
#define SLICE_COUNT 1000

uniform sampler2D	uMaterialDiffuseTex;
uniform sampler2D	uMaterialSpecularTex;
uniform sampler2D   uOpaqueDepthTex;
uniform sampler2D	uWaveletOpacityMap1;
uniform sampler2D	uWaveletOpacityMap2;
uniform sampler2D	uWaveletOpacityMap3;
uniform sampler2D	uWaveletOpacityMap4;
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

void main()
{
	float depth = texelFetch(uOpaqueDepthTex, ivec2(gl_FragCoord.xy), 0).r;
	if (depth != 0.0 && gl_FragCoord.z > depth) discard;

	depth = _linearizeDepth(gl_FragCoord.z, uNearPlane, uFarPlane) - 0.0;

	vec4 map1 = texelFetch(uWaveletOpacityMap1, ivec2(gl_FragCoord.xy), 0);
	vec4 map2 = texelFetch(uWaveletOpacityMap2, ivec2(gl_FragCoord.xy), 0);

	float opticalDepth = 0.0;
	opticalDepth += map1.x * phi_integral(depth);
	opticalDepth += map1.y * psi_integral(depth, 0, 0);
	opticalDepth += map1.z * psi_integral(depth, 1, 0);
	opticalDepth += map1.w * psi_integral(depth, 1, 1);
	opticalDepth += map2.x * psi_integral(depth, 2, 0);
	opticalDepth += map2.y * psi_integral(depth, 2, 1);
	opticalDepth += map2.z * psi_integral(depth, 2, 2);
	opticalDepth += map2.w * psi_integral(depth, 2, 3);

	if (uWOITCoeffNum == 16)
	{
		vec4 map3 = texelFetch(uWaveletOpacityMap3, ivec2(gl_FragCoord.xy), 0);
		vec4 map4 = texelFetch(uWaveletOpacityMap4, ivec2(gl_FragCoord.xy), 0);
		opticalDepth += map3.x * psi_integral(depth, 3, 0);
		opticalDepth += map3.y * psi_integral(depth, 3, 1);
		opticalDepth += map3.z * psi_integral(depth, 3, 2);
		opticalDepth += map3.w * psi_integral(depth, 3, 3);
		opticalDepth += map4.x * psi_integral(depth, 3, 4);
		opticalDepth += map4.y * psi_integral(depth, 3, 5);
		opticalDepth += map4.z * psi_integral(depth, 3, 6);
		opticalDepth += map4.w * psi_integral(depth, 3, 7);
	}

	float transmittance = exp(-opticalDepth);

	vec3 reflectColor = computeReflectColor();
	_outTransparencyColor.rgb = transmittance * uCoverage * reflectColor;
	_outTransparencyColor.a = transmittance * uCoverage;
}