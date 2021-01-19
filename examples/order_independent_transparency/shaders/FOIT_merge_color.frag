#version 460 core
#include "common.glsl"

uniform sampler2D uOpaqueColorTex;
uniform sampler2D uTranslucentColorTex;

layout(binding = 0, OIT_FLT_PRECISION) uniform image2DArray uFourierOpacityMaps;
layout(binding = 1, rgba8ui) uniform uimage2DArray uQuantizedFourierOpacityMaps;

void main()
{
	ivec2 uv = ivec2(gl_FragCoord.xy);
	vec3  opaqueColor = texelFetch(uOpaqueColorTex, uv, 0).rgb;
	vec4  translucentColor = texelFetch(uTranslucentColorTex, uv, 0).rgba;

#ifndef FOIT_ENABLE_QUANTIZATION
	float totalOpticalDepth = 0.5 * imageLoad(uFourierOpacityMaps, ivec3(uv, 0)).y;
#else
	float totalOpticalDepth =  0.5 * dequantize(imageLoad(uQuantizedFourierOpacityMaps, ivec3(uv, 0)).y);
#endif

	float totalTransmittance = exp(-totalOpticalDepth);

	vec3 finalColor = mix(translucentColor.rgb / (translucentColor.a + 1e-5), opaqueColor, totalTransmittance);
	//vec3 finalColor = mix(translucentColor.rgb, opaqueColor, totalTransmittance);

	gl_FragColor = vec4(finalColor, 1.0);
}