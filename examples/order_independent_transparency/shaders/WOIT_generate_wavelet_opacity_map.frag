#version 460 core
#extension GL_NV_fragment_shader_interlock : require

#include "common.glsl"
#include "WOIT_common.glsl"

uniform sampler2D		uOpaqueDepthTex;
uniform sampler2D		uPsiLutTex;

uniform float		uCoverage;
uniform float		uNearPlane;
uniform float		uFarPlane;

layout(location = 0) in float _inFragDepth;

layout(location = 0) out float _outTotalAbsorbance;

#ifndef WOIT_ENABLE_QUANTIZATION
	layout(location = 1) out vec4 _outWaveletCoeffs1;
	layout(location = 2) out vec4 _outWaveletCoeffs2;
	layout(location = 3) out vec4 _outWaveletCoeffs3;
	layout(location = 4) out vec4 _outWaveletCoeffs4;
#else
	layout(binding = 0, rgba8ui) coherent uniform uimage2D	uWaveletCoeffsMap1;
	layout(binding = 1, rgba8ui) coherent uniform uimage2D	uWaveletCoeffsMap2;
	layout(binding = 2, rgba8ui) coherent uniform uimage2D	uWaveletCoeffsMap3;
	layout(binding = 3, rgba8ui) coherent uniform uimage2D	uWaveletCoeffsMap4;
#endif



float meyer_basis(float d, int i)
{
	int indexX = int(BASIS_SLICE_COUNT * d);
	indexX = min(max(indexX, 0), BASIS_SLICE_COUNT - 1);
	int indexY = i;
	return 2 * BASIS_SCALE * texelFetch(uPsiLutTex, ivec2(indexX, indexY), 0).r - BASIS_SCALE;
}

float basisFunc(float x, int i)
{
	float result = 0;
	int indexJ = (i != 0) ? int(floor(log2(float(i)))) : 0;
	int indexK = int(i - pow(2, indexJ));

#if BASIS_TYPE == FOURIER_BASIS
	if (i == 0) 
	{
		result = 2;
	}
	else
	{
		int k = (i - 1) / 2 + 1;
		result = (i % 2 == 1) ? 2*cos(2*PI*k*x) : 2*sin(2*PI*k*x);
	}
#elif BASIS_TYPE == HAAR_BASIS
		if (i == 0)
			result = haar_phi(x);
		else
			result = haar_psi(x, indexJ, indexK);
#elif BASIS_TYPE == MEYER_BASIS
	result = meyer_basis(x, i);
#endif

	return result;
}

void main()
{
	float opaqueDepth = texelFetch(uOpaqueDepthTex, ivec2(gl_FragCoord.xy), 0).r;
	if (opaqueDepth != 0.0 && gl_FragCoord.z > opaqueDepth) discard;

	float depth = _linearizeDepth(gl_FragCoord.z, uNearPlane, uFarPlane);
	//float depth = gl_FragCoord.z;

	float absorbance = -log(1.0 - uCoverage + 1e-5);

	_outTotalAbsorbance = absorbance;

#ifndef WOIT_ENABLE_QUANTIZATION
	int basisIndex = 0;
	if (BASIS_NUM >= 4)
	{
		_outWaveletCoeffs1.r = basisFunc(depth, basisIndex++) * absorbance;
		_outWaveletCoeffs1.g = basisFunc(depth, basisIndex++) * absorbance;
		_outWaveletCoeffs1.b = basisFunc(depth, basisIndex++) * absorbance;
		_outWaveletCoeffs1.a = basisFunc(depth, basisIndex++) * absorbance;
	}

	if (BASIS_NUM >= 8)
	{
		_outWaveletCoeffs2.r = basisFunc(depth, basisIndex++) * absorbance;
		_outWaveletCoeffs2.g = basisFunc(depth, basisIndex++) * absorbance;
		_outWaveletCoeffs2.b = basisFunc(depth, basisIndex++) * absorbance;
		_outWaveletCoeffs2.a = basisFunc(depth, basisIndex++) * absorbance;
	}

	if (BASIS_NUM >= 12)
	{
		_outWaveletCoeffs3.r = basisFunc(depth, basisIndex++) * absorbance;
		_outWaveletCoeffs3.g = basisFunc(depth, basisIndex++) * absorbance;
		_outWaveletCoeffs3.b = basisFunc(depth, basisIndex++) * absorbance;
		_outWaveletCoeffs3.a = basisFunc(depth, basisIndex++) * absorbance;
	}

	if (BASIS_NUM >= 16)
	{
		_outWaveletCoeffs4.r = basisFunc(depth, basisIndex++) * absorbance;
		_outWaveletCoeffs4.g = basisFunc(depth, basisIndex++) * absorbance;
		_outWaveletCoeffs4.b = basisFunc(depth, basisIndex++) * absorbance;
		_outWaveletCoeffs4.a = basisFunc(depth, basisIndex++) * absorbance;
	}
#else
	beginInvocationInterlockNV();

	int basisIndex = 0;
	if (BASIS_NUM >= 4)
	{
		vec4 coeffs = dequantize(imageLoad(uWaveletCoeffsMap1, ivec2(gl_FragCoord.xy)));
		coeffs = expandFuncMiuReverse(coeffs, _IntervalMin, _IntervalMax, _Mu);
		coeffs.x += basisFunc(depth, basisIndex++) * absorbance;
		coeffs.y += basisFunc(depth, basisIndex++) * absorbance;
		coeffs.z += basisFunc(depth, basisIndex++) * absorbance;
		coeffs.w += basisFunc(depth, basisIndex++) * absorbance;
		coeffs = expandFuncMiu(coeffs, _IntervalMin, _IntervalMax, _Mu);
		imageStore(uWaveletCoeffsMap1, ivec2(gl_FragCoord.xy), quantize(coeffs));
	}

	if (BASIS_NUM >= 8)
	{
		vec4 coeffs = dequantize(imageLoad(uWaveletCoeffsMap2, ivec2(gl_FragCoord.xy)));
		coeffs = expandFuncMiuReverse(coeffs, _IntervalMin, _IntervalMax, _Mu);
		coeffs.x += basisFunc(depth, basisIndex++) * absorbance;
		coeffs.y += basisFunc(depth, basisIndex++) * absorbance;
		coeffs.z += basisFunc(depth, basisIndex++) * absorbance;
		coeffs.w += basisFunc(depth, basisIndex++) * absorbance;
		coeffs = expandFuncMiu(coeffs, _IntervalMin, _IntervalMax, _Mu);
		imageStore(uWaveletCoeffsMap2, ivec2(gl_FragCoord.xy), quantize(coeffs));
	}

	if (BASIS_NUM >= 12)
	{
		vec4 coeffs = dequantize(imageLoad(uWaveletCoeffsMap3, ivec2(gl_FragCoord.xy)));
		coeffs = expandFuncMiuReverse(coeffs, _IntervalMin, _IntervalMax, _Mu);
		coeffs.x += basisFunc(depth, basisIndex++) * absorbance;
		coeffs.y += basisFunc(depth, basisIndex++) * absorbance;
		coeffs.z += basisFunc(depth, basisIndex++) * absorbance;
		coeffs.w += basisFunc(depth, basisIndex++) * absorbance;
		coeffs = expandFuncMiu(coeffs, _IntervalMin, _IntervalMax, _Mu);
		imageStore(uWaveletCoeffsMap3, ivec2(gl_FragCoord.xy), quantize(coeffs));
	}

	if (BASIS_NUM >= 16)
	{
		vec4 coeffs = dequantize(imageLoad(uWaveletCoeffsMap4, ivec2(gl_FragCoord.xy)));
		coeffs = expandFuncMiuReverse(coeffs, _IntervalMin, _IntervalMax, _Mu);
		coeffs.x += basisFunc(depth, basisIndex++) * absorbance;
		coeffs.y += basisFunc(depth, basisIndex++) * absorbance;
		coeffs.z += basisFunc(depth, basisIndex++) * absorbance;
		coeffs.w += basisFunc(depth, basisIndex++) * absorbance;
		coeffs = expandFuncMiu(coeffs, _IntervalMin, _IntervalMax, _Mu);
		imageStore(uWaveletCoeffsMap4, ivec2(gl_FragCoord.xy), quantize(coeffs));
	}

	endInvocationInterlockNV();
#endif
}