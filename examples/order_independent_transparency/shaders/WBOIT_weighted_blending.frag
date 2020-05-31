#version 460 core
#include "compute_phong_shading.glsl"

#define WEIGHTED_BLENDING 0
#define AVERAGE_BLENDING  1

uniform sampler2D	uMaterialDiffuse;
uniform sampler2D	uMaterialSpecular;
uniform sampler2D   uOpaqueDepthTex;

uniform vec3	uViewPos = vec3(0.0);
uniform vec3	uDiffuseColor;
uniform vec3	uTransmittance = vec3(0.0);
uniform float	uCoverage;
uniform int		uWeightingStragety = WEIGHTED_BLENDING;

layout(location = 0) in vec3 _inPositionW;
layout(location = 1) in vec3 _inNormalW;
layout(location = 2) in vec2 _inTexCoord;

layout(location = 0) out vec4 _outAccumulatedTranslucentColor;
layout(location = 1) out vec3 _outAccumulatedTransmittance;

#include "compute_reflection_color.glsl"

void main()
{
	float depth = texelFetch(uOpaqueDepthTex, ivec2(gl_FragCoord.xy), 0).r;
	if (depth != 0.0 && gl_FragCoord.z > depth) discard;

	float t_average = (uTransmittance.x + uTransmittance.y + uTransmittance.z) / 3.0;
	float weight = 0.0;
	if (uWeightingStragety == WEIGHTED_BLENDING)
	{
		weight = pow(10.0 * (1.0 - 0.99 * gl_FragCoord.z) * uCoverage * (1.0 - t_average) , 3.0);
		weight = clamp(weight, 0.01, 30.0);
	}
	else if (uWeightingStragety == AVERAGE_BLENDING)
	{
		weight = 1.0;
	}
	
	_outAccumulatedTranslucentColor.rgb = weight * uCoverage * computeReflectColor();
	_outAccumulatedTranslucentColor.a = weight * uCoverage * (1.0 - t_average);

	_outAccumulatedTransmittance = -log(1.0 - uCoverage + uCoverage * uTransmittance + 1e-5);
}