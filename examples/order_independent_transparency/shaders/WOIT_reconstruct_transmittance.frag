#version 460 core
#extension GL_NV_shader_atomic_float : require
#extension GL_ARB_shader_image_load_store : require

#include "common.glsl"
#include "compute_phong_shading.glsl"
#include "WOIT_common.glsl"

uniform sampler2D		uMaterialDiffuseTex;
uniform sampler2D		uMaterialSpecularTex;
uniform sampler2D		uOpaqueDepthTex;
uniform sampler2D		uPsiIntegralLutTex;
uniform sampler2D		uWaveletCoeffsMap1;
uniform sampler2D		uWaveletCoeffsMap2;
uniform sampler2D		uWaveletCoeffsMap3;
uniform sampler2D		uWaveletCoeffsMap4;

uniform vec3	uViewPos = vec3(0.0);
uniform vec3	uDiffuseColor;
uniform float	uCoverage;
uniform float	uNearPlane;
uniform float	uFarPlane;

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
		result *= SIGMA_K(k, BASIS_NUM / 2);
	}
#elif BASIS_TYPE == HAAR_BASIS
	if (i == 0)
		result = haar_phi_integral(x);
	else
		result = haar_psi_integral(x, indexJ, indexK);
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
	//depth = gl_FragCoord.z;

//#ifdef ENABLE_DEPTH_REMAPPING
//	float nearestSurfaceZ = imageLoad(uSurfaceZImage, ivec2(gl_FragCoord.xy)).x;
//	float farthestSurfaceZ = imageLoad(uSurfaceZImage, ivec2(gl_FragCoord.xy)).y;
//	depth = remap(depth, nearestSurfaceZ, farthestSurfaceZ, 0.1, 0.9);
//#endif

	float opticalDepth = 0.0;
	int basisIndex = 0;

	if (BASIS_NUM >= 4)
	{
		vec4 coeffs = texelFetch(uWaveletCoeffsMap1, ivec2(gl_FragCoord.xy), 0);
		opticalDepth += basisIntegralFunc(depth, basisIndex++) * coeffs.x;
		opticalDepth += basisIntegralFunc(depth, basisIndex++) * coeffs.y;
		opticalDepth += basisIntegralFunc(depth, basisIndex++) * coeffs.z;
		opticalDepth += basisIntegralFunc(depth, basisIndex++) * coeffs.w;
	}

	if (BASIS_NUM >= 8)
	{
		vec4 coeffs = texelFetch(uWaveletCoeffsMap2, ivec2(gl_FragCoord.xy), 0);
		opticalDepth += basisIntegralFunc(depth, basisIndex++) * coeffs.x;
		opticalDepth += basisIntegralFunc(depth, basisIndex++) * coeffs.y;
		opticalDepth += basisIntegralFunc(depth, basisIndex++) * coeffs.z;
		opticalDepth += basisIntegralFunc(depth, basisIndex++) * coeffs.w;
	}

	if (BASIS_NUM >= 12)
	{
		vec4 coeffs = texelFetch(uWaveletCoeffsMap3, ivec2(gl_FragCoord.xy), 0);
		opticalDepth += basisIntegralFunc(depth, basisIndex++) * coeffs.x;
		opticalDepth += basisIntegralFunc(depth, basisIndex++) * coeffs.y;
		opticalDepth += basisIntegralFunc(depth, basisIndex++) * coeffs.z;
		opticalDepth += basisIntegralFunc(depth, basisIndex++) * coeffs.w;
	}

	if (BASIS_NUM >= 16)
	{
		vec4 coeffs = texelFetch(uWaveletCoeffsMap4, ivec2(gl_FragCoord.xy), 0);
		opticalDepth += basisIntegralFunc(depth, basisIndex++) * coeffs.x;
		opticalDepth += basisIntegralFunc(depth, basisIndex++) * coeffs.y;
		opticalDepth += basisIntegralFunc(depth, basisIndex++) * coeffs.z;
		opticalDepth += basisIntegralFunc(depth, basisIndex++) * coeffs.w;
	}

	float transmittance = exp(-opticalDepth);

	vec3 reflectColor = computeReflectColor();
	_outTransparencyColor.rgb = transmittance * uCoverage * reflectColor;
	_outTransparencyColor.a = transmittance * uCoverage;
}