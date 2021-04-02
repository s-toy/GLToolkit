#version 460 core
#extension GL_NV_fragment_shader_interlock : require
#extension GL_NV_shader_atomic_float : require

#include "common.glsl"

uniform sampler2D		uOpaqueDepthTex;
uniform sampler2D		uPsiLutTex;
uniform sampler3D		uDefaultQuantizerParamsImage;
uniform sampler2DArray	uOptimalQuantizerParamsImage;

#include "WOIT_quantization.glsl"

uniform float		uCoverage;
uniform float		uNearPlane;
uniform float		uFarPlane;
uniform int			uScaleSize;
uniform int			uTileSize;
uniform int			uTileCountW;
uniform int			uTileCountH;
uniform int			uTileCountD;

layout(location = 0) in float _inFragDepth;

layout(location = 0) out float _outTotalAbsorbance;

layout(binding = 0, WOIT_FLT_PRECISION) coherent uniform image2DArray	uWaveletOpacityMaps;
layout(binding = 1, r8ui)				coherent uniform uimage2DArray	uQuantizedWaveletOpacityMaps;

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

	float absorbance = -log(1.0 - uCoverage + 1e-5);

	_outTotalAbsorbance = absorbance;

	float coeffsIncr[BASIS_NUM];
	for (int i = 0; i < BASIS_NUM; ++i)
	{
		coeffsIncr[i] = basisFunc(depth, i) * absorbance;
	}

	vec3 texCoord;
	texCoord.xy = gl_FragCoord.xy / (vec2(uTileCountW, uTileCountH) * uTileSize * uScaleSize);
	int basisNumPerTile = (BASIS_NUM - 1) / uTileCountD + 1;

	beginInvocationInterlockNV();

	for (int i = 0; i < BASIS_NUM; ++i)
	{
		if (abs(coeffsIncr[i]) < 0.001) continue;

		//texCoord.z = (float(i) + 0.5) / float(basisNumPerTile * uTileCountD);
		texCoord.z = (float(i / basisNumPerTile) + 0.5) / float(uTileCountD);
		//texCoord.z = depth;
		vec3 uniformQuantizerParams = texture(uDefaultQuantizerParamsImage, texCoord).xyz;

#if defined(WOIT_ENABLE_QUANTIZATION) || defined(WOIT_ENABLE_QERROR_CALCULATION)
	{
	#if QUANTIZATION_METHOD == UNIFORM_QUANTIZATION
		float coeff = dequantize(imageLoad(uQuantizedWaveletOpacityMaps, ivec3(gl_FragCoord.xy, i)).r, uniformQuantizerParams.x, uniformQuantizerParams.z);
		coeff = expandFuncMiuReverse(coeff, _IntervalMin, _IntervalMax, _Mu);
		coeff += coeffsIncr[i];
		coeff = expandFuncMiu(coeff, _IntervalMin, _IntervalMax, _Mu);
		imageStore(uQuantizedWaveletOpacityMaps, ivec3(gl_FragCoord.xy, i), ivec4(quantize(coeff, uniformQuantizerParams.x, uniformQuantizerParams.z), 0, 0, 0));
	#elif QUANTIZATION_METHOD == LLOYD_MAX_QUANTIZATION
		float coeff = dequantize(imageLoad(uQuantizedWaveletOpacityMaps, ivec3(gl_FragCoord.xy, i)).r, texCoord.xy);
		coeff += coeffsIncr[i];
		imageStore(uQuantizedWaveletOpacityMaps, ivec3(gl_FragCoord.xy, i), ivec4(quantize(coeff, texCoord.xy), 0, 0, 0));
	#endif
	}
#endif

#if !defined(WOIT_ENABLE_QUANTIZATION) || defined(WOIT_ENABLE_QERROR_CALCULATION)
	{
		float coeff = imageLoad(uWaveletOpacityMaps, ivec3(gl_FragCoord.xy, i)).r;
		coeff += coeffsIncr[i];
		imageStore(uWaveletOpacityMaps, ivec3(gl_FragCoord.xy, i), vec4(coeff, 0, 0, 0));
	}
#endif
	} //end for

	endInvocationInterlockNV();
}