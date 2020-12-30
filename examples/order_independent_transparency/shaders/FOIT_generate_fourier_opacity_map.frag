#version 460 core
#extension GL_ARB_fragment_shader_interlock : require

#include "common.glsl"

uniform sampler2D   uOpaqueDepthTex;
uniform float		uCoverage;
uniform float		uNearPlane;
uniform float		uFarPlane;
uniform int			uFOITCoeffNum;

layout(location = 0) in float _inFragDepth;

layout(location = 0) out float _outEmptyRenderTexture;

layout(binding = 0, FOIT_FLT_PRECISION) uniform image2DArray uFourierOpacityMaps;
layout(binding = 1, rgba8ui) uniform uimage2DArray uQuantizedFourierOpacityMaps;

void main()
{
	_outEmptyRenderTexture = 0;

	float opaqueDepth = texelFetch(uOpaqueDepthTex, ivec2(gl_FragCoord.xy), 0).r;
	if (opaqueDepth != 0.0 && gl_FragCoord.z > opaqueDepth) discard;

	float depth = _linearizeDepth(gl_FragCoord.z, uNearPlane, uFarPlane);

	float absorbance = -log(1.0 - uCoverage + 1e-5);
	//float opticalDepth = 5000.0 * uCoverage * (depth);
	//float absorbance = gl_FrontFacing ? -opticalDepth : opticalDepth;
	float a0 = 2 * absorbance;
	float sin2, cos2, sin4, cos4, sin6, cos6, sin8, cos8, sin10, cos10, sin12, cos12, sin14, cos14;

	beginInvocationInterlockARB();

	{
		cos2 = cos(2 * PI * depth);
		sin2 = sin(2 * PI * depth);
		float a1 = a0 * cos2;
		float b1 = a0 * sin2;

#ifndef FOIT_ENABLE_QUANTIZATION
		vec4 fourierOpacityData1 = imageLoad(uFourierOpacityMaps, ivec3(gl_FragCoord.xy, 0));
		fourierOpacityData1 += vec4(0, a0, a1, b1);
		imageStore(uFourierOpacityMaps, ivec3(gl_FragCoord.xy, 0), fourierOpacityData1);
#else
		vec4 quantizedFourierOpacityData1 = dequantizeVec4(imageLoad(uQuantizedFourierOpacityMaps, ivec3(gl_FragCoord.xy, 0)));
		quantizedFourierOpacityData1 += vec4(0, a0, a1, b1);
		imageStore(uQuantizedFourierOpacityMaps, ivec3(gl_FragCoord.xy, 0), quantizeVec4(quantizedFourierOpacityData1));
#endif
	}

	if (uFOITCoeffNum > 3)
	{
		cos4 = cos2 * cos2 - sin2 * sin2;
		sin4 = 2 * cos2 * sin2;
		cos6 = cos4 * cos2 - sin4 * sin2;
		sin6 = sin4 * cos2 + cos4 * sin2;
		float a2 = a0 * cos4;
		float b2 = a0 * sin4;
		float a3 = a0 * cos6;
		float b3 = a0 * sin6; 

#ifndef FOIT_ENABLE_QUANTIZATION
		vec4 fourierOpacityData2 = imageLoad(uFourierOpacityMaps, ivec3(gl_FragCoord.xy, 1));
		fourierOpacityData2 += vec4(a2, b2, a3, b3);
		imageStore(uFourierOpacityMaps, ivec3(gl_FragCoord.xy, 1), fourierOpacityData2);
#else
		vec4 quantizedFourierOpacityData2 = dequantizeVec4(imageLoad(uQuantizedFourierOpacityMaps, ivec3(gl_FragCoord.xy, 1)));
		quantizedFourierOpacityData2 += vec4(a2, b2, a3, b3);
		imageStore(uQuantizedFourierOpacityMaps, ivec3(gl_FragCoord.xy, 1), quantizeVec4(quantizedFourierOpacityData2));
#endif
	}

	if (uFOITCoeffNum > 7)
	{
		cos8 = cos4 * cos4 - sin4 * sin4;
		sin8 = 2 * cos4 * sin4;
		cos10 = cos6 * cos4 - sin6 * sin4;
		sin10 = sin6 * cos4 + cos6 * sin4;
		float a4 = a0 * cos8;
		float b4 = a0 * sin8;
		float a5 = a0 * cos10;
		float b5 = a0 * sin10;

#ifndef FOIT_ENABLE_QUANTIZATION
		vec4 fourierOpacityData3 = imageLoad(uFourierOpacityMaps, ivec3(gl_FragCoord.xy, 2));
		fourierOpacityData3 += vec4(a4, b4, a5, b5);
		imageStore(uFourierOpacityMaps, ivec3(gl_FragCoord.xy, 2), fourierOpacityData3);
#else
		vec4 quantizedFourierOpacityData3 = dequantizeVec4(imageLoad(uQuantizedFourierOpacityMaps, ivec3(gl_FragCoord.xy, 2)));
		quantizedFourierOpacityData3 += vec4(a4, b4, a5, b5);
		imageStore(uQuantizedFourierOpacityMaps, ivec3(gl_FragCoord.xy, 2), quantizeVec4(quantizedFourierOpacityData3));
#endif
	}

	if (uFOITCoeffNum > 13)
	{
		cos12 = cos6 * cos6 - sin6 * sin6;
		sin12 = 2 * sin6 * cos6;
		cos14 = cos8 * cos6 - sin8 * sin6;
		sin14 = cos8 * sin6 + sin8 * cos6;
		float a6 = a0 * cos12;
		float b6 = a0 * sin12;
		float a7 = a0 * cos14;
		float b7 = a0 * sin14;

#ifndef FOIT_ENABLE_QUANTIZATION
		vec4 fourierOpacityData4 = imageLoad(uFourierOpacityMaps, ivec3(gl_FragCoord.xy, 3));
		fourierOpacityData4 += vec4(a6, b6, a7, b7);
		imageStore(uFourierOpacityMaps, ivec3(gl_FragCoord.xy, 3), fourierOpacityData4);
#else
		vec4 quantizedFourierOpacityData4 = dequantizeVec4(imageLoad(uQuantizedFourierOpacityMaps, ivec3(gl_FragCoord.xy, 3)));
		quantizedFourierOpacityData4 += vec4(a6, b6, a7, b7);
		imageStore(uQuantizedFourierOpacityMaps, ivec3(gl_FragCoord.xy, 3), quantizeVec4(quantizedFourierOpacityData4));
#endif
	}

	endInvocationInterlockARB();
}