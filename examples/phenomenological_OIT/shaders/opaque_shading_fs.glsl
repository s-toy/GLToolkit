#version 430 core

uniform sampler2D uMaterialDiffuse;
uniform sampler2D uMaterialSpecular;

uniform vec3 uViewPos = vec3(0.0);

layout(location = 0) in vec3 _inPositionW;
layout(location = 1) in vec3 _inNormalW;
layout(location = 2) in vec2 _inTexCoord;

layout(location = 0) out vec3 _outFragColor;
layout(location = 1) out float _outFragDepth;

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
	Material.Diffuse = texture(uMaterialDiffuse, _inTexCoord).rgb;
	Material.Specular = texture(uMaterialSpecular, _inTexCoord).rgb;
	Material.Shinness = 32.0;

	vec3 FinalColor = vec3(0.0);
	FinalColor += computePhongShading4ParallelLight(_inPositionW, NormalW, ViewDirW, SParallelLight(vec3(0.7), vec3(1.0, 1.0, 1.0)), Material);
	FinalColor += computePhongShading4ParallelLight(_inPositionW, NormalW, ViewDirW, SParallelLight(vec3(0.7), vec3(-1.0, 1.0, -1.0)), Material);

	_outFragColor = FinalColor;
	_outFragDepth = gl_FragCoord.z;
}