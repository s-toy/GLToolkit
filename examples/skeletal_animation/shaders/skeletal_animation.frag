#version 460
#extension GL_ARB_separate_shader_objects : enable

uniform sampler2D uMaterialDiffuseTex;
uniform sampler2D uMaterialSpecularTex;

uniform vec3 uViewPos = vec3(0.0);

layout(location = 0) in vec3 _inPositionW;
layout(location = 1) in vec3 _inNormalW;
layout(location = 2) in vec2 _inTexCoord;

layout(location = 0) out vec4 _outFragColor;

struct SParallelLight { vec3 Color; vec3 Direction; };
struct SMaterial { vec3 Diffuse; vec3 Specular; float Shinness; };

vec3 computePhongShading4ParallelLight(vec3 vPositionW, vec3 vNormalW, vec3 vViewDir, SParallelLight vLight, SMaterial vMaterial)
{
	vec3 AmbientColor = 0.2 * vLight.Color * vMaterial.Diffuse;
	vec3 DiffuseColor = vLight.Color * vMaterial.Diffuse * max(dot(vNormalW, vLight.Direction), 0.0);
	vec3 ReflectDir = normalize(reflect(-vLight.Direction, _inNormalW));
	vec3 SpecularColor = vLight.Color * vMaterial.Specular * pow(max(dot(vViewDir, ReflectDir), 0.0), vMaterial.Shinness);

	return AmbientColor + DiffuseColor + SpecularColor;
}

void main()
{
	vec3 ViewDirW = normalize(uViewPos - _inPositionW);
	vec3 NormalW = normalize(_inNormalW);

	SMaterial Material;
	Material.Diffuse = texture(uMaterialDiffuseTex, _inTexCoord).rgb;
	Material.Specular = texture(uMaterialSpecularTex, _inTexCoord).rgb;
	Material.Shinness = 32.0;

	vec3 FinalColor = vec3(0.0);
	FinalColor += computePhongShading4ParallelLight(_inPositionW, NormalW, ViewDirW, SParallelLight(vec3(0.7), vec3(1.0, 1.0, 1.0)), Material);
	FinalColor += computePhongShading4ParallelLight(_inPositionW, NormalW, ViewDirW, SParallelLight(vec3(0.7), vec3(-1.0, 1.0, -1.0)), Material);

	_outFragColor = vec4(FinalColor, 1.0);
}