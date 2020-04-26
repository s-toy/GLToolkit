#version 430 core

uniform sampler2D   uOpaqueDepthTex;
uniform float		uCoverage;

layout(location = 0) out float	_outMomentB0;
layout(location = 1) out vec4	_outMoments;

void main()
{
	float opaqueDepth = texelFetch(uOpaqueDepthTex, ivec2(gl_FragCoord.xy), 0).r;
	if (opaqueDepth != 0.0 && gl_FragCoord.z > opaqueDepth) discard;

	float absorbance = -log(1.0 - uCoverage);
	float depth = gl_FragCoord.z;
	float depth_pow2 = depth * depth;
	float depth_pow4 = depth_pow2 * depth_pow2;

	_outMomentB0 = absorbance;
	_outMoments = vec4(depth, depth_pow2, depth_pow2 * depth, depth_pow4) * absorbance;
}