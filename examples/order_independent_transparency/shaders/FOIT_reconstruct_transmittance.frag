#version 460 core
#include "common.glsl"
#include "compute_phong_shading.glsl"

#define PI 3.1415926

uniform sampler2D	uMaterialDiffuseTex;
uniform sampler2D	uMaterialSpecularTex;
uniform sampler2D   uOpaqueDepthTex;
uniform sampler2D	uFourierOpacityMap1;
uniform sampler2D	uFourierOpacityMap2;
uniform sampler2D	uFourierOpacityMap3;
uniform sampler2D	uFourierOpacityMap4;

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

	vec4 xx_a0_a1_b1 = texelFetch(uFourierOpacityMap1, ivec2(gl_FragCoord.xy), 0);

	float cos2 = cos(2 * PI * depth);
	float sin2 = sin(2 * PI * depth);
	float cos4, cos6, cos8, cos10, cos12, cos14;
	float sin4, sin6, sin8, sin10, sin12, sin14;

	float a0 = xx_a0_a1_b1.y;
	float a1 = xx_a0_a1_b1.z;
	float b1 = xx_a0_a1_b1.w;

	float opticalDepth = 0.5 * a0 * depth;
	opticalDepth += (a1 / (2*PI)) * sin2;
	opticalDepth += (b1 / (2*PI)) * (1-cos2);

	if (uFOITCoeffNum > 3)
	{
		cos4 = cos2 * cos2 - sin2 * sin2;
		sin4 = 2 * cos2 * sin2;
		cos6 = cos4 * cos2 - sin4 * sin2;
		sin6 = sin4 * cos2 + cos4 * sin2;

		vec4 a2_b2_a3_b3 = texelFetch(uFourierOpacityMap2, ivec2(gl_FragCoord.xy), 0);
		float a2 = a2_b2_a3_b3.x;
		float b2 = a2_b2_a3_b3.y;
		float a3 = a2_b2_a3_b3.z;
		float b3 = a2_b2_a3_b3.w;

		opticalDepth += (a2 / (4*PI)) * sin4;
		opticalDepth += (b2 / (4*PI)) * (1-cos4);
		opticalDepth += (a3 / (6*PI)) * sin6;
		opticalDepth += (b3 / (6*PI)) * (1-cos6);
	}

	if (uFOITCoeffNum > 7)
	{
		cos8 = cos4 * cos4 - sin4 * sin4;
		sin8 = 2 * cos4 * sin4;
		cos10 = cos6 * cos4 - sin6 * sin4;
		sin10 = sin6 * cos4 + cos6 * sin4;

		vec4 a4_b4_a5_b5 = texelFetch(uFourierOpacityMap3, ivec2(gl_FragCoord.xy), 0);
		float a4 = a4_b4_a5_b5.x;
		float b4 = a4_b4_a5_b5.y;
		float a5 = a4_b4_a5_b5.z;
		float b5 = a4_b4_a5_b5.w;

		opticalDepth += (a4 / (8*PI)) * sin8;
		opticalDepth += (b4 / (8*PI)) * (1-cos8);
		opticalDepth += (a5 / (10*PI)) * sin10;
		opticalDepth += (b5 / (10*PI)) * (1-cos10);
	}

	if (uFOITCoeffNum > 11)
	{
		cos12 = cos6 * cos6 - sin6 * sin6;
		sin12 = 2 * sin6 * cos6;
		cos14 = cos8 * cos6 - sin8 * sin6;
		sin14 = cos8 * sin6 + sin8 * cos6;
		
		vec4 a6_b6_a7_b7 = texelFetch(uFourierOpacityMap4, ivec2(gl_FragCoord.xy), 0);
		float a6 = a6_b6_a7_b7.x;
		float b6 = a6_b6_a7_b7.y;
		float a7 = a6_b6_a7_b7.z;
		float b7 = a6_b6_a7_b7.w;

		opticalDepth += (a6 / (12*PI)) * sin12;
		opticalDepth += (b6 / (12*PI)) * (1-cos12);
		opticalDepth += (a7 / (14*PI)) * sin14;
		opticalDepth += (b7 / (14*PI)) * (1-cos14);
	}

	float transmittance = exp(-opticalDepth);

	vec3 reflectColor = computeReflectColor();
	_outTransparencyColor.rgb = transmittance * uCoverage * reflectColor;
	_outTransparencyColor.a = transmittance * uCoverage;
}