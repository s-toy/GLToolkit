#version 460 core
#include "common.glsl"
#include "compute_phong_shading.glsl"

#define PI 3.1415926
//#define ENABLE_SIGMA_AVERAGING 

#ifdef ENABLE_SIGMA_AVERAGING
	#define SIGMA_K(k, n) (sin(k*PI/n) / (k*PI/n))
#else
	#define SIGMA_K(k, n) 1
#endif

uniform sampler2D	uMaterialDiffuseTex;
uniform sampler2D	uMaterialSpecularTex;
uniform sampler2D   uOpaqueDepthTex;

layout(binding = 0, OIT_FLT_PRECISION) uniform image2DArray uFourierOpacityMaps;
layout(binding = 1, rgba8ui) uniform uimage2DArray uQuantizedFourierOpacityMaps;

uniform vec3	uViewPos = vec3(0.0);
uniform vec3	uDiffuseColor;
uniform float	uCoverage;
uniform float	uNearPlane;
uniform float	uFarPlane;
uniform int		uFOITCoeffNum;

layout(location = 0) in vec3 _inPositionW;
layout(location = 1) in vec3 _inNormalW;
layout(location = 2) in vec2 _inTexCoord;

layout(location = 0) out vec4 _outTransparencyColor;

#include "compute_reflection_color.glsl"

void main()
{
	float depth = texelFetch(uOpaqueDepthTex, ivec2(gl_FragCoord.xy), 0).r;
	if (depth != 0.0 && gl_FragCoord.z > depth) discard;

	depth = _linearizeDepth(gl_FragCoord.z, uNearPlane, uFarPlane);

#ifndef FOIT_ENABLE_QUANTIZATION
	vec4 xx_a0_a1_b1 = imageLoad(uFourierOpacityMaps, ivec3(gl_FragCoord.xy, 0));
#else
	vec4 xx_a0_a1_b1 = dequantizeVec4(imageLoad(uQuantizedFourierOpacityMaps, ivec3(gl_FragCoord.xy, 0)));
#endif

	float cos2 = cos(2 * PI * depth);
	float sin2 = sin(2 * PI * depth);
	float cos4, cos6, cos8, cos10, cos12, cos14;
	float sin4, sin6, sin8, sin10, sin12, sin14;

	float a0 = xx_a0_a1_b1.y;
	float a1 = xx_a0_a1_b1.z;
	float b1 = xx_a0_a1_b1.w;

	float N = uFOITCoeffNum / 2;
	float opticalDepth = 0.5 * a0 * depth;
	opticalDepth += (a1 / (2*PI)) * sin2 * SIGMA_K(1, N);
	opticalDepth += (b1 / (2*PI)) * (1-cos2) * SIGMA_K(1, N);

		cos4 = cos2 * cos2 - sin2 * sin2;
		sin4 = 2 * cos2 * sin2;
		cos6 = cos4 * cos2 - sin4 * sin2;
		sin6 = sin4 * cos2 + cos4 * sin2;

#ifndef FOIT_ENABLE_QUANTIZATION
		vec4 a2_b2_a3_b3 = imageLoad(uFourierOpacityMaps, ivec3(gl_FragCoord.xy, 1));
#else
		vec4 a2_b2_a3_b3 = dequantizeVec4(imageLoad(uQuantizedFourierOpacityMaps, ivec3(gl_FragCoord.xy, 1)));
#endif

		float a2 = a2_b2_a3_b3.x;
		float b2 = a2_b2_a3_b3.y;
		float a3 = a2_b2_a3_b3.z;
		float b3 = a2_b2_a3_b3.w;

		opticalDepth += (a2 / (4*PI)) * sin4 * SIGMA_K(1, N);
		opticalDepth += (b2 / (4*PI)) * (1-cos4) * SIGMA_K(1, N);
		opticalDepth += (a3 / (6*PI)) * sin6 * SIGMA_K(1, N);
		opticalDepth += (b3 / (6*PI)) * (1-cos6) * SIGMA_K(1, N);

	float transmittance = exp(-opticalDepth);

	vec3 reflectColor = computeReflectColor();
	_outTransparencyColor.rgb = transmittance * uCoverage * reflectColor;
	_outTransparencyColor.a = transmittance * uCoverage;
}