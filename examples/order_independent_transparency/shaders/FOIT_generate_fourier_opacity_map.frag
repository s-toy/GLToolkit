#version 460 core
#include "common.glsl"
#include "reconstruction_config.glsl"

#define PI 3.1415926

uniform sampler2D   uOpaqueDepthTex;
uniform float		uCoverage;
uniform float		uNearPlane;
uniform float		uFarPlane;

layout(location = 0) in float _inFragDepth;

layout(location = 0) out vec4 _outFourierOpacityMap1;
layout(location = 1) out vec4 _outFourierOpacityMap2;
layout(location = 2) out vec4 _outFourierOpacityMap3;
layout(location = 3) out vec4 _outFourierOpacityMap4;

void main()
{
	float opaqueDepth = texelFetch(uOpaqueDepthTex, ivec2(gl_FragCoord.xy), 0).r;
	if (opaqueDepth != 0.0 && gl_FragCoord.z > opaqueDepth) discard;

	float absorbance = -log(1.0 - uCoverage + 1e-5);
	float depth = _linearizeDepth(gl_FragCoord.z, uNearPlane, uFarPlane);

	float a0 = 2 * absorbance;
	float a1 = a0 * cos(2 * PI * depth);
	float b1 = a0 * sin(2 * PI * depth);

	float a2 = a0 * cos(4 * PI * depth);
	float b2 = a0 * sin(4 * PI * depth);
	float a3 = a0 * cos(6 * PI * depth);
	float b3 = a0 * sin(6 * PI * depth);

	float a4 = a0 * cos(8 * PI * depth);
	float b4 = a0 * sin(8 * PI * depth);
	float a5 = a0 * cos(10 * PI * depth);
	float b5 = a0 * sin(10 * PI * depth);

	float a6 = a0 * cos(12 * PI * depth);
	float b6 = a0 * sin(12 * PI * depth);
	float a7 = a0 * cos(14 * PI * depth);
	float b7 = a0 * sin(14 * PI * depth);

	_outFourierOpacityMap1 = vec4(0, a0, a1, b1);
	_outFourierOpacityMap2 = vec4(a2, b2, a3, b3);
	_outFourierOpacityMap3 = vec4(a4, b4, a5, b5);
	_outFourierOpacityMap4 = vec4(a6, b6, a7, b7);
}