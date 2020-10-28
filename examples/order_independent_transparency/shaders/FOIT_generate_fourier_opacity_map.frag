#version 460 core
#include "common.glsl"

uniform sampler2D   uOpaqueDepthTex;
uniform float		uCoverage;
uniform float		uNearPlane;
uniform float		uFarPlane;
uniform int			uFOITCoeffNum;

layout(location = 0) in float _inFragDepth;

layout(location = 0) out vec4 _outFourierOpacityMap1;
layout(location = 1) out vec4 _outFourierOpacityMap2;
layout(location = 2) out vec4 _outFourierOpacityMap3;
layout(location = 3) out vec4 _outFourierOpacityMap4;

void main()
{
	float opaqueDepth = texelFetch(uOpaqueDepthTex, ivec2(gl_FragCoord.xy), 0).r;
	if (opaqueDepth != 0.0 && gl_FragCoord.z > opaqueDepth) discard;

	float depth = _linearizeDepth(gl_FragCoord.z, uNearPlane, uFarPlane);

	float absorbance = -log(1.0 - uCoverage + 1e-5);
	//float opticalDepth = 100.0 * uCoverage * depth;
	//float absorbance = gl_FrontFacing ? -opticalDepth : opticalDepth;

	float cos2 = cos(2 * PI * depth);
	float sin2 = sin(2 * PI * depth);
	float a0 = 2 * absorbance;
	float a1 = a0 * cos2;
	float b1 = a0 * sin2;
	_outFourierOpacityMap1 = vec4(0, a0, a1, b1);
	if (uFOITCoeffNum <= 3) return;
	
	float cos4 = cos2 * cos2 - sin2 * sin2;
	float sin4 = 2 * cos2 * sin2;
	float cos6 = cos4 * cos2 - sin4 * sin2;
	float sin6 = sin4 * cos2 + cos4 * sin2;
	float a2 = a0 * cos4;
	float b2 = a0 * sin4;
	float a3 = a0 * cos6;
	float b3 = a0 * sin6; 
	_outFourierOpacityMap2 = vec4(a2, b2, a3, b3);
	if (uFOITCoeffNum <= 7) return;

	float cos8 = cos4 * cos4 - sin4 * sin4;
	float sin8 = 2 * cos4 * sin4;
	float cos10 = cos6 * cos4 - sin6 * sin4;
	float sin10 = sin6 * cos4 + cos6 * sin4;
	float a4 = a0 * cos8;
	float b4 = a0 * sin8;
	float a5 = a0 * cos10;
	float b5 = a0 * sin10;
	_outFourierOpacityMap3 = vec4(a4, b4, a5, b5);
	if (uFOITCoeffNum <= 11) return;

	float cos12 = cos6 * cos6 - sin6 * sin6;
	float sin12 = 2 * sin6 * cos6;
	float cos14 = cos8 * cos6 - sin8 * sin6;
	float sin14 = cos8 * sin6 + sin8 * cos6;
	float a6 = a0 * cos12;
	float b6 = a0 * sin12;
	float a7 = a0 * cos14;
	float b7 = a0 * sin14;
	_outFourierOpacityMap4 = vec4(a6, b6, a7, b7);
}