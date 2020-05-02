#version 430 core
#include "MomentMath.glsl"
#include "compute_phong_shading.glsl"

#define POWER_MOMENT_4		0
#define POWER_MOMENT_6		1
#define POWER_MOMENT_8		2

uniform int uReconstructionStrategy = POWER_MOMENT_4;

uniform sampler2D	uMaterialDiffuse;
uniform sampler2D	uMaterialSpecular;
uniform sampler2D   uOpaqueDepthTex;

uniform sampler2D	uMomentB0Tex;
uniform sampler2D	uMomentsTex;
uniform sampler2D	uExtraMomentsTex;

uniform vec3	uViewPos = vec3(0.0);
uniform vec3	uDiffuseColor;
uniform float	uCoverage;

layout(location = 0) in vec3 _inPositionW;
layout(location = 1) in vec3 _inNormalW;
layout(location = 2) in vec2 _inTexCoord;

layout(location = 0) out vec4 _outTransparencyColor;

#include "compute_reflection_color.glsl"

void main()
{
	float depth = texelFetch(uOpaqueDepthTex, ivec2(gl_FragCoord.xy), 0).r;
	if (depth != 0.0 && gl_FragCoord.z > depth) discard;

	float transmittance_at_depth = 0.0;
	
	if (uReconstructionStrategy == POWER_MOMENT_4)
	{
		float b_0 = texelFetch(uMomentB0Tex, ivec2(gl_FragCoord.xy), 0).x;
		if (b_0 < 0.00100050033f) discard;
		vec4  b_1234 = texelFetch(uMomentsTex, ivec2(gl_FragCoord.xy), 0).xyzw;
		vec2  b_even = b_1234.yw;
		vec2  b_odd = b_1234.xz;
		b_even /= b_0;
		b_odd /= b_0;

		const vec4 bias_vector = vec4(0, 0.375, 0, 0.375);
		float depth = 2.0 * gl_FragCoord.z - 1.0;
		float moment_bias = 5e-7; //recommended bias from http://momentsingraphics.de/Media/I3D2018/Muenstermann2018-MBOITSupplementary.pdf
		float overestimation = 0.25;

		transmittance_at_depth = computeTransmittanceAtDepthFrom4PowerMoments(b_0, b_even, b_odd, depth, moment_bias, overestimation, bias_vector);
	}
	else if (uReconstructionStrategy == POWER_MOMENT_6)
	{
		float b_0 = texelFetch(uMomentB0Tex, ivec2(gl_FragCoord.xy), 0).x;
		if (b_0 < 0.00100050033f) discard;
		vec4  b_1234 = texelFetch(uMomentsTex, ivec2(gl_FragCoord.xy), 0).xyzw;
		vec2  b_56 = texelFetch(uExtraMomentsTex, ivec2(gl_FragCoord.xy), 0).xy;
		vec3  b_even = vec3(b_1234.yw, b_56.y);
		vec3  b_odd  = vec3(b_1234.xz, b_56.x);
		b_even /= b_0;
		b_odd /= b_0;

		const float bias_vector[6] = { 0, 0.48, 0, 0.451, 0, 0.45 };
		float depth = 2.0 * gl_FragCoord.z - 1.0;
		float moment_bias = 5e-6;
		float overestimation = 0.25;

		transmittance_at_depth = computeTransmittanceAtDepthFrom6PowerMoments(b_0, b_even, b_odd, depth, moment_bias, overestimation, bias_vector);
	}
	else if (uReconstructionStrategy == POWER_MOMENT_8)
	{
		float b_0 = texelFetch(uMomentB0Tex, ivec2(gl_FragCoord.xy), 0).x;
		if (b_0 < 0.00100050033f) discard;
		vec4  b_1234 = texelFetch(uMomentsTex, ivec2(gl_FragCoord.xy), 0).xyzw;
		vec4  b_5678 = texelFetch(uExtraMomentsTex, ivec2(gl_FragCoord.xy), 0).xyzw;
		vec4  b_even = vec4(b_1234.yw, b_5678.yw);
		vec4  b_odd  = vec4(b_1234.xz, b_5678.xz);
		b_even /= b_0;
		b_odd /= b_0;

		const float bias_vector[8] = { 0, 0.75, 0, 0.67666666666666664, 0, 0.63, 0, 0.60030303030303034 };
		float depth = 2.0 * gl_FragCoord.z - 1.0;
		float moment_bias = 5e-5;
		float overestimation = 0.25;

		transmittance_at_depth = computeTransmittanceAtDepthFrom8PowerMoments(b_0, b_even, b_odd, depth, moment_bias, overestimation, bias_vector);
	}

	vec3 reflectColor = computeReflectColor();
	_outTransparencyColor.rgb = transmittance_at_depth * uCoverage * reflectColor;
	_outTransparencyColor.a = transmittance_at_depth * uCoverage;
}