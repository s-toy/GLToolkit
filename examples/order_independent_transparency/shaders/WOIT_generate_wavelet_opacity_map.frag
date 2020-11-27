#version 460 core
#include "common.glsl"

uniform sampler2D   uOpaqueDepthTex;
uniform float		uCoverage;
uniform float		uNearPlane;
uniform float		uFarPlane;
uniform int			uWOITCoeffNum;

layout(location = 0) in float _inFragDepth;

layout(location = 0) out vec4 _outWaveletOpacityMap1;
layout(location = 1) out vec4 _outWaveletOpacityMap2;
layout(location = 2) out vec4 _outWaveletOpacityMap3;
layout(location = 3) out vec4 _outWaveletOpacityMap4;

float phi(float x)
{
	return 1;
}

float psi(float x, float j, float k)
{
	float value = pow(2.0f, j / 2.0f);
	//value *= value;

	const float intervalLength = 1.0f / pow(2.0f, j + 1);
	const float intervalMin = 2 * k * intervalLength;
	const float intervalMid = intervalMin + intervalLength;
	const float intervalMax = intervalMid + intervalLength;

	if (x >= intervalMin && x < intervalMid)
		return value;
	else if (x >= intervalMid && x < intervalMax)
		return -value;
	else
		return 0;
}

void main()
{
	float opaqueDepth = texelFetch(uOpaqueDepthTex, ivec2(gl_FragCoord.xy), 0).r;
	if (opaqueDepth != 0.0 && gl_FragCoord.z > opaqueDepth) discard;

	float depth = _linearizeDepth(gl_FragCoord.z, uNearPlane, uFarPlane);

	float absorbance = -log(1.0 - uCoverage + 1e-5);
	//float opticalDepth = 50.0 * uCoverage * (depth);
	//float absorbance = gl_FrontFacing ? -opticalDepth : opticalDepth;

	float phi00 = absorbance * phi(depth);
	float psi00 =  absorbance * psi(depth, 0, 0);
	float psi10 =  absorbance * psi(depth, 1, 0);
	float psi11 =  absorbance * psi(depth, 1, 1);
	float psi20 =  absorbance * psi(depth, 2, 0);
	float psi21 =  absorbance * psi(depth, 2, 1);
	float psi22 =  absorbance * psi(depth, 2, 2);
	float psi23 =  absorbance * psi(depth, 2, 3);
	_outWaveletOpacityMap1 = vec4(phi00, psi00, psi10, psi11);
	_outWaveletOpacityMap2 = vec4(psi20, psi21, psi22, psi23);
	if (uWOITCoeffNum <= 8) return;

	float psi30 =  absorbance * psi(depth, 3, 0);
	float psi31 =  absorbance * psi(depth, 3, 1);
	float psi32 =  absorbance * psi(depth, 3, 2);
	float psi33 =  absorbance * psi(depth, 3, 3);
	float psi34 =  absorbance * psi(depth, 3, 4);
	float psi35 =  absorbance * psi(depth, 3, 5);
	float psi36 =  absorbance * psi(depth, 3, 6);
	float psi37 =  absorbance * psi(depth, 3, 7);
	_outWaveletOpacityMap3 = vec4(psi30, psi31, psi32, psi33);
	_outWaveletOpacityMap4 = vec4(psi34, psi35, psi36, psi37);
}