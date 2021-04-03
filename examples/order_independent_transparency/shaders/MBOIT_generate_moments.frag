#version 460 core
#extension GL_ARB_fragment_shader_interlock : require
#include "common.glsl"
#include "reconstruction_config.glsl"
#include "moment_math.glsl"

uniform sampler2D   uOpaqueDepthTex;
uniform float		uCoverage;
uniform float		uNearPlane;
uniform float		uFarPlane;
uniform vec4		uWrappingZoneParameters;

layout(binding = 1, rgba32f) uniform image2D uMomentsImage;
layout(location = 0) in float _inFragDepth;

layout(location = 0) out float	_outMomentB0;
layout(location = 1) out vec4	_outMomentB1234;
layout(location = 2) out vec4	_outMomentB5678;

void main()
{
	float opaqueDepth = texelFetch(uOpaqueDepthTex, ivec2(gl_FragCoord.xy), 0).r;
	if (opaqueDepth != 0.0 && gl_FragCoord.z > opaqueDepth) discard;

	float depth = _linearizeDepth(gl_FragCoord.z, uNearPlane, uFarPlane);

	float absorbance = -log(1.0 - uCoverage + 1e-5);

	depth = 2.0 * depth - 1.0;

	_outMomentB0 = absorbance;

	float depth_pow2 = depth * depth;
	float depth_pow4 = depth_pow2 * depth_pow2;
	float depth_pow6 = depth_pow2 * depth_pow4;

	_outMomentB1234 = vec4(depth, depth_pow2, depth_pow2 * depth, depth_pow4) * absorbance;
	if (NUM_MOMENTS > 4) _outMomentB5678 = vec4(depth * depth_pow4, depth_pow2 * depth_pow4, depth * depth_pow6, depth_pow4 * depth_pow4) * absorbance;

	//beginInvocationInterlockARB();

	//vec4 Moments = imageLoad(uMomentsImage, ivec2(gl_FragCoord.xy)).xyzw;
	//Moments += vec4(depth, depth_pow2, depth_pow2 * depth, depth_pow4) * absorbance;
	//imageStore(uMomentsImage, ivec2(gl_FragCoord.xy), Moments); 

	//endInvocationInterlockARB();
}