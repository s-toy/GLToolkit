#version 430 core

uniform sampler2D   uOpaqueDepthTex;
uniform float		uCoverage;

layout(location = 0) out float _outMomentB0;

void main()
{
	float depth = texelFetch(uOpaqueDepthTex, ivec2(gl_FragCoord.xy), 0).r;
	if (depth != 0.0 && gl_FragCoord.z > depth) discard;

	_outMomentB0 = -log(1.0 - uCoverage);
}