#version 460 core
#extension GL_ARB_fragment_shader_interlock : require
#include "reconstruction_config.glsl"
#include "moment_math.glsl"

uniform sampler2D   uOpaqueDepthTex;
uniform float		uCoverage;
uniform vec4		uWrappingZoneParameters;

layout(binding = 1, rgba32f) uniform image2D uMomentsImage;
layout(location = 0) in float _inFragDepth;

layout(location = 0) out float	_outMomentB0;

void main()
{
	float opaqueDepth = texelFetch(uOpaqueDepthTex, ivec2(gl_FragCoord.xy), 0).r;
	if (opaqueDepth != 0.0 && gl_FragCoord.z > opaqueDepth) discard;

	float absorbance = -log(1.0 - uCoverage + 1e-5);
	float depth = 2.0 * gl_FragCoord.z - 1.0; //_inFragDepth;

	_outMomentB0 = absorbance;

#if !TRIGONOMETRIC
	float depth_pow2 = depth * depth;
	float depth_pow4 = depth_pow2 * depth_pow2;
	float depth_pow6 = depth_pow2 * depth_pow4;

	beginInvocationInterlockARB();

	vec4 Moments = imageLoad(uMomentsImage, ivec2(gl_FragCoord.xy)).xyzw;
	Moments += vec4(depth, depth_pow2, depth_pow2 * depth, depth_pow4) * absorbance;
	imageStore(uMomentsImage, ivec2(gl_FragCoord.xy), Moments); 

	endInvocationInterlockARB();
#else
	float phase = fma(depth, uWrappingZoneParameters.y, uWrappingZoneParameters.y);
	vec2 circle_point;
	sincos(phase, circle_point.y, circle_point.x);
	vec2 circle_point_pow2 = Multiply(circle_point, circle_point);

	_outMoments = vec4(circle_point, circle_point_pow2) * absorbance;
	_outExtraMoments = vec4(Multiply(circle_point, circle_point_pow2), Multiply(circle_point_pow2, circle_point_pow2)) * absorbance;
#endif
}