#version 460 core
#include "compute_phong_shading.glsl"

uniform sampler2D uMaterialDiffuseTex;
uniform sampler2D uMaterialSpecularTex;

uniform vec3 uMaterialDiffuse;
uniform vec3 uMaterialSpecular;

uniform vec3 uViewPos = vec3(0.0);

layout(location = 0) in vec3 _inPositionW;
layout(location = 1) in vec3 _inNormalW;
layout(location = 2) in vec2 _inTexCoord;

layout(location = 0) out vec3 _outFragColor;

void main()
{
	vec3 ViewDirW = normalize(uViewPos - _inPositionW);
	vec3 NormalW = normalize(_inNormalW);

	SMaterial Material;
	Material.Diffuse = vec3(0.6);// texture(uMaterialDiffuseTex, _inTexCoord).rgb;
	Material.Specular = vec3(0.3);// texture(uMaterialSpecularTex, _inTexCoord).rgb;
	Material.Shinness = 32.0;

	vec3 color = vec3(0.0);
	color += computePhongShading4ParallelLight(_inPositionW, NormalW, ViewDirW, SParallelLight(vec3(0.7), vec3(1.0, 1.0, 1.0)), Material);
	color += computePhongShading4ParallelLight(_inPositionW, NormalW, ViewDirW, SParallelLight(vec3(0.7), vec3(-1.0, 1.0, -1.0)), Material);

	_outFragColor = color;
}