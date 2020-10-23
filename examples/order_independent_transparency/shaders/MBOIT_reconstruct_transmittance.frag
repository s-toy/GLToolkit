#version 460 core
#include "common.glsl"
#include "moment_math.glsl"
#include "compute_phong_shading.glsl"
#include "reconstruction_config.glsl"

uniform sampler2D	uMaterialDiffuse;
uniform sampler2D	uMaterialSpecular;
uniform sampler2D   uOpaqueDepthTex;

uniform sampler2D	uMomentB0Tex;
layout(binding = 1, rgba32f) uniform image2D uMomentsImage;

uniform vec3	uViewPos = vec3(0.0);
uniform vec3	uDiffuseColor;
uniform float	uCoverage;
uniform float	uNearPlane;
uniform float	uFarPlane;
uniform vec4	uWrappingZoneParameters;

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
	float b_0 = texelFetch(uMomentB0Tex, ivec2(gl_FragCoord.xy), 0).x;
	if (b_0 < 0.00100050033f) discard;

	float overestimation = 0.25;
	depth = _linearizeDepth(gl_FragCoord.z, uNearPlane, uFarPlane);
	depth = 2.0 * depth - 1.0;

#if NUM_MOMENTS == 4
 #if !TRIGONOMETRIC
	vec4 b_1234 = imageLoad(uMomentsImage, ivec2(gl_FragCoord.xy)).xyzw;
	vec2 b_even = b_1234.yw;
	vec2 b_odd = b_1234.xz;
	b_even /= b_0;
	b_odd /= b_0;
	const vec4 bias_vector = vec4(0, 0.375, 0, 0.375);
	float moment_bias = 5e-7; //recommended bias from http://momentsingraphics.de/Media/I3D2018/Muenstermann2018-MBOITSupplementary.pdf
	transmittance_at_depth = computeTransmittanceAtDepthFrom4PowerMoments(b_0, b_even, b_odd, depth, moment_bias, overestimation, bias_vector);
 #else
	vec4 b_tmp = texelFetch(uMomentsTex, ivec2(gl_FragCoord.xy), 0).xyzw;
	vec2 trig_b[2];
	trig_b[0] = b_tmp.xy;
	trig_b[1] = b_tmp.zw;
	trig_b[0] /= b_0;
	trig_b[1] /= b_0;
	float moment_bias = 4e-4;
	transmittance_at_depth = computeTransmittanceAtDepthFrom2TrigonometricMoments(b_0, trig_b, depth, moment_bias, overestimation, uWrappingZoneParameters);
 #endif
#elif NUM_MOMENTS == 6
 #if !TRIGONOMETRIC
	vec4 b_1234 = texelFetch(uMomentsTex, ivec2(gl_FragCoord.xy), 0).xyzw;
	vec2 b_56 = texelFetch(uExtraMomentsTex, ivec2(gl_FragCoord.xy), 0).xy;
	vec3 b_even = vec3(b_1234.yw, b_56.y);
	vec3 b_odd  = vec3(b_1234.xz, b_56.x);
	b_even /= b_0;
	b_odd /= b_0;
	const float bias_vector[6] = { 0, 0.48, 0, 0.451, 0, 0.45 };
	float moment_bias = 5e-6;
	transmittance_at_depth = computeTransmittanceAtDepthFrom6PowerMoments(b_0, b_even, b_odd, depth, moment_bias, overestimation, bias_vector);
 #else
 	vec4 b_tmp = texelFetch(uMomentsTex, ivec2(gl_FragCoord.xy), 0).xyzw;
	vec2 b_tmp2 = texelFetch(uExtraMomentsTex, ivec2(gl_FragCoord.xy), 0).xy;
	vec2 trig_b[3];
	trig_b[0] = b_tmp.xy;
	trig_b[1] = b_tmp.zw;
	trig_b[2] = b_tmp2.xy;
	trig_b[0] /= b_0;
	trig_b[1] /= b_0;
	trig_b[2] /= b_0;
	float moment_bias = 2e-4;
	transmittance_at_depth = computeTransmittanceAtDepthFrom3TrigonometricMoments(b_0, trig_b, depth, moment_bias, overestimation, uWrappingZoneParameters);
 #endif
#elif NUM_MOMENTS == 8
 #if !TRIGONOMETRIC
	vec4 b_1234 = texelFetch(uMomentsTex, ivec2(gl_FragCoord.xy), 0).xyzw;
	vec4 b_5678 = texelFetch(uExtraMomentsTex, ivec2(gl_FragCoord.xy), 0).xyzw;
	vec4 b_even = vec4(b_1234.yw, b_5678.yw);
	vec4 b_odd  = vec4(b_1234.xz, b_5678.xz);
	b_even /= b_0;
	b_odd /= b_0;
	const float bias_vector[8] = { 0, 0.75, 0, 0.67666666666666664, 0, 0.63, 0, 0.60030303030303034 };
	float moment_bias = 5e-5;
	transmittance_at_depth = computeTransmittanceAtDepthFrom8PowerMoments(b_0, b_even, b_odd, depth, moment_bias, overestimation, bias_vector);
 #else
  	vec4 b_tmp = texelFetch(uMomentsTex, ivec2(gl_FragCoord.xy), 0).xyzw;
	vec4 b_tmp2 = texelFetch(uExtraMomentsTex, ivec2(gl_FragCoord.xy), 0).xyzw;
	vec2 trig_b[4];
	trig_b[0] = b_tmp.xy;
	trig_b[1] = b_tmp.zw;
	trig_b[2] = b_tmp2.xy;
	trig_b[3] = b_tmp2.zw;
	trig_b[0] /= b_0;
	trig_b[1] /= b_0;
	trig_b[2] /= b_0;
	trig_b[3] /= b_0;
	float moment_bias = 2e-4;
	transmittance_at_depth = computeTransmittanceAtDepthFrom4TrigonometricMoments(b_0, trig_b, depth, moment_bias, overestimation, uWrappingZoneParameters);
 #endif
#endif

	vec3 reflectColor = computeReflectColor();
	_outTransparencyColor.rgb = transmittance_at_depth * uCoverage * reflectColor;
	_outTransparencyColor.a = transmittance_at_depth * uCoverage;
}