#version 460 core
#extension GL_NV_shader_atomic_float : require

#include "common.glsl"
#include "WOIT_common.glsl"

layout(binding = 0, WOIT_FLT_PRECISION) uniform image2DArray	uWaveletOpacityMaps;
layout(binding = 1, r8ui)				uniform uimage2DArray	uQuantizedWaveletOpacityMaps;
layout(binding = 7, r32f)				uniform image2D			uTotalQuantizationErrorImage;

uniform sampler2D uDefaultQuantizerParamsImage;

uniform int		uScaleSize;
uniform int		uTileSize;
uniform int		uTileCountW;
uniform int		uTileCountH;

layout(location = 0) out float _outQuantizationError;

void main()
{
	//ivec2 tileCoord = ivec2(gl_FragCoord.xy) / (uTileSize * uScaleSize);
	//vec3 uniformQuantizerParams = texelFetch(uDefaultQuantizerParamsImage, tileCoord, 0).xyz;

	vec2 texCoord = gl_FragCoord.xy / (vec2(uTileCountW, uTileCountH) * uTileSize * uScaleSize);
	vec3 uniformQuantizerParams = texture(uDefaultQuantizerParamsImage, texCoord).xyz;

	float error = 0;

	for (int i = 0; i < BASIS_NUM; ++i)
	{
		float coeff = imageLoad(uWaveletOpacityMaps, ivec3(gl_FragCoord.xy, i)).r;
		float coeffQuan = dequantize(imageLoad(uQuantizedWaveletOpacityMaps, ivec3(gl_FragCoord.xy, i)).r, uniformQuantizerParams.x, uniformQuantizerParams.z);
		coeffQuan = expandFuncMiuReverse(coeffQuan, _IntervalMin, _IntervalMax, _Mu);
		error += (coeff - coeffQuan) * (coeff - coeffQuan);
	}

	error /= BASIS_NUM;

	imageAtomicAdd(uTotalQuantizationErrorImage, ivec2(0,0), error);

	_outQuantizationError = error;
}