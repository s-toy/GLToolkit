#version 430 core
#include "reconstruction_strategy.glsl"

uniform sampler2D   uOpaqueDepthTex;
uniform float		uCoverage;

layout(location = 0) out float	_outMomentB0;
layout(location = 1) out vec4	_outMoments;
layout(location = 2) out vec4	_outExtraMoments;

layout(location = 0) in float _inFragDepth;

void main()
{
	float opaqueDepth = texelFetch(uOpaqueDepthTex, ivec2(gl_FragCoord.xy), 0).r;
	if (opaqueDepth != 0.0 && gl_FragCoord.z > opaqueDepth) discard;

	float absorbance = -log(1.0 - uCoverage + 1e-5);
	float depth = 2.0 * gl_FragCoord.z - 1.0; //_inFragDepth;
	float depth_pow2 = depth * depth;
	float depth_pow4 = depth_pow2 * depth_pow2;
	float depth_pow6 = depth_pow2 * depth_pow4;

	_outMomentB0 = absorbance;
	_outMoments = vec4(depth, depth_pow2, depth_pow2 * depth, depth_pow4) * absorbance;

	if (uReconstructionStrategy == POWER_MOMENT_6 || uReconstructionStrategy == POWER_MOMENT_8)
		_outExtraMoments = vec4(depth*depth_pow4, depth_pow6, depth*depth_pow6, depth_pow2*depth_pow6) * absorbance;
}