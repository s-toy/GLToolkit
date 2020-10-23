#version 460 core
#include "common.glsl"
#include "compute_phong_shading.glsl"
#include "reconstruction_config.glsl"

#define PI 3.1415926

uniform sampler2D	uMaterialDiffuse;
uniform sampler2D	uMaterialSpecular;
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
	vec4 a2_b2_a3_b3 = texelFetch(uFourierOpacityMap2, ivec2(gl_FragCoord.xy), 0);
	vec4 a4_b4_a5_b5 = texelFetch(uFourierOpacityMap3, ivec2(gl_FragCoord.xy), 0);
	vec4 a6_b6_a7_b7 = texelFetch(uFourierOpacityMap4, ivec2(gl_FragCoord.xy), 0);

	float a0 = xx_a0_a1_b1.y;
	float a1 = xx_a0_a1_b1.z;
	float b1 = xx_a0_a1_b1.w;

	float a2 = a2_b2_a3_b3.x;
	float b2 = a2_b2_a3_b3.y;
	float a3 = a2_b2_a3_b3.z;
	float b3 = a2_b2_a3_b3.w;

	float a4 = a4_b4_a5_b5.x;
	float b4 = a4_b4_a5_b5.y;
	float a5 = a4_b4_a5_b5.z;
	float b5 = a4_b4_a5_b5.w;

	float a6 = a6_b6_a7_b7.x;
	float b6 = a6_b6_a7_b7.y;
	float a7 = a6_b6_a7_b7.z;
	float b7 = a6_b6_a7_b7.w;

	float opticalDepth = 0.5 * a0 * depth;
	opticalDepth += (a1 / (2*PI)) * sin(2*PI*depth);
	opticalDepth += (b1 / (2*PI)) * (1 - cos(2*PI*depth));

	opticalDepth += (a2 / (4*PI)) * sin(4*PI*depth);
	opticalDepth += (b2 / (4*PI)) * (1 - cos(4*PI*depth));
	opticalDepth += (a3 / (6*PI)) * sin(6*PI*depth);
	opticalDepth += (b3 / (6*PI)) * (1 - cos(6*PI*depth));

	opticalDepth += (a4 / (8*PI)) * sin(8*PI*depth);
	opticalDepth += (b4 / (8*PI)) * (1 - cos(8*PI*depth));
	opticalDepth += (a5 / (10*PI)) * sin(10*PI*depth);
	opticalDepth += (b5 / (10*PI)) * (1 - cos(10*PI*depth));

	opticalDepth += (a6 / (12*PI)) * sin(12*PI*depth);
	opticalDepth += (b6 / (12*PI)) * (1 - cos(12*PI*depth));
	opticalDepth += (a7 / (14*PI)) * sin(14*PI*depth);
	opticalDepth += (b7 / (14*PI)) * (1 - cos(14*PI*depth));

	float transmittance = exp(-opticalDepth);

	vec3 reflectColor = computeReflectColor();
	_outTransparencyColor.rgb = transmittance * uCoverage * reflectColor;
	_outTransparencyColor.a = transmittance * uCoverage;
}