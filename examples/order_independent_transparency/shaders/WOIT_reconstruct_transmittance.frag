#version 460 core
#extension GL_NV_shader_atomic_float : require
#extension GL_ARB_shader_image_load_store : require

#include "common.glsl"
#include "WOIT_common.glsl"
#include "compute_phong_shading.glsl"

layout(binding = 0, WOIT_FLT_PRECISION) uniform image2DArray uWaveletOpacityMaps;
layout(binding = 1, r8ui) uniform uimage2DArray	uQuantizedWaveletOpacityMaps;
layout(binding = 5, rg16f)	 uniform image2D		uSurfaceZImage;

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

float meyer_basis_integral(float d, int i)
{
	int indexX = int(BASIS_SLICE_COUNT * d);
	indexX = min(max(indexX, 0), BASIS_SLICE_COUNT - 1);
	int indexY = i;
	return 2 * BASIS_SCALE * texelFetch(uPsiIntegralLutTex, ivec2(indexX, indexY), 0).r - BASIS_SCALE;
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

	#ifdef USING_DIRECT_PROJECTION
		if (i == 0)
			result = haar_phi(x);
		else
			result = haar_psi(x, indexJ, indexK);
	#else
		if (i == 0)
			result = haar_phi_integral(x);
		else
			result = haar_psi_integral(x, indexJ, indexK);
	#endif

#elif BASIS_TYPE == MEYER_BASIS
	result = meyer_basis_integral(x, i);
#endif
	return result;
}

void main()
{
	float depth = texelFetch(uOpaqueDepthTex, ivec2(gl_FragCoord.xy), 0).r;
	if (depth != 0.0 && gl_FragCoord.z > depth) discard;

	depth = _linearizeDepth(gl_FragCoord.z, uNearPlane, uFarPlane);

#ifdef ENABLE_DEPTH_REMAPPING
	float nearestSurfaceZ = imageLoad(uSurfaceZImage, ivec2(gl_FragCoord.xy)).x;
	float farthestSurfaceZ = imageLoad(uSurfaceZImage, ivec2(gl_FragCoord.xy)).y;
	depth = remap(depth, nearestSurfaceZ, farthestSurfaceZ, 0.1, 0.9);
#endif

	float opticalDepth = 0.0;

	float basisIntegral[BASIS_NUM];
	for (int i = 0; i < BASIS_NUM; ++i)
	{
		basisIntegral[i] = basisIntegralFunc(depth, i);
	}

	for (int i = 0; i < BASIS_NUM; ++i)
	{
#ifndef WOIT_ENABLE_QUANTIZATION
		float coeff = imageLoad(uWaveletOpacityMaps, ivec3(gl_FragCoord.xy, i)).r;
#else
		float coeff = dequantize(imageLoad(uQuantizedWaveletOpacityMaps, ivec3(gl_FragCoord.xy, i)).r);
#endif
		opticalDepth += coeff * basisIntegral[i];
	}

	float transmittance = exp(-opticalDepth);

	vec3 reflectColor = computeReflectColor();
	_outTransparencyColor.rgb = transmittance * uCoverage * reflectColor;
	_outTransparencyColor.a = transmittance * uCoverage;
}