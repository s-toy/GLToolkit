#version 460 core
#extension GL_NV_shader_atomic_float : require

#include "common.glsl"

uniform sampler3D		uDefaultQuantizerParamsImage;
uniform sampler2DArray	uOptimalQuantizerParamsImage;

#include "WOIT_quantization.glsl"

layout(binding = 0, WOIT_FLT_PRECISION) uniform image2DArray	uWaveletOpacityMaps;
layout(binding = 1, r8ui)				uniform uimage2DArray	uQuantizedWaveletOpacityMaps;
layout(binding = 7, r32f)				uniform image2D			uTotalQuantizationErrorImage;

uniform int				uScaleSize;
uniform int				uTileSize;
uniform int				uTileCountW;
uniform int				uTileCountH;
uniform int				uTileCountD;

layout(location = 0) out float _outQuantizationError;

void main()
{
	vec3 texCoord;
	texCoord.xy = gl_FragCoord.xy / (vec2(uTileCountW, uTileCountH) * uTileSize * uScaleSize);
	int basisNumPerTile = (BASIS_NUM - 1) / uTileCountD + 1;

	float error = 0;
	float coeff2 = 0;

	for (int i = 0; i < BASIS_NUM; ++i)
	{
		//texCoord.z = (float(i) + 0.5) / float(basisNumPerTile * uTileCountD);
		texCoord.z = (float(i / basisNumPerTile) + 0.5) / float(uTileCountD);
		vec3 uniformQuantizerParams = texture(uDefaultQuantizerParamsImage, texCoord).xyz;

		float coeff = imageLoad(uWaveletOpacityMaps, ivec3(gl_FragCoord.xy, i)).r;

	#if QUANTIZATION_METHOD == UNIFORM_QUANTIZATION
		float coeffQuan = dequantize(imageLoad(uQuantizedWaveletOpacityMaps, ivec3(gl_FragCoord.xy, i)).r, uniformQuantizerParams.x, uniformQuantizerParams.z);
		coeffQuan = expandFuncMiuReverse(coeffQuan, _IntervalMin, _IntervalMax, _Mu);
	#elif QUANTIZATION_METHOD == LLOYD_MAX_QUANTIZATION
		float coeffQuan = dequantize(imageLoad(uQuantizedWaveletOpacityMaps, ivec3(gl_FragCoord.xy, i)).r, texCoord.xy);
	#endif

		error += (coeff - coeffQuan) * (coeff - coeffQuan);
		coeff2 += coeff * coeff;
	}

	error /= BASIS_NUM;
	coeff2 /= BASIS_NUM;

	imageAtomicAdd(uTotalQuantizationErrorImage, ivec2(0, 0), error);
	imageAtomicAdd(uTotalQuantizationErrorImage, ivec2(0, 1), coeff2);

	_outQuantizationError = error;
}