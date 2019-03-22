#version 460 core

uniform sampler2D uMaterialDiffuse1;
uniform sampler2D uMaterialSpecular1;
uniform float uMaterialShinness = 32.0;

uniform vec3 uLightColor = vec3(1.0);

uniform vec3 uViewPos = vec3(0.0);

in vec3 _PositionW;
in vec3 _NormalW;
in vec2 _TexCoord;

out vec4 _outFragColor;

struct SParallelLight
{
	vec3 Color;
	vec3 Direction;
};

struct SMaterial
{
	vec3 Diffuse;
	vec3 Specular;
	float Shinness;
};

vec3 computePhongShading4ParallelLight(vec3 vPositionW, vec3 vNormalW, vec3 vViewDir, SParallelLight vLight, SMaterial vMaterial)
{
	vec3 AmbientColor = 0.2 * vLight.Color * vMaterial.Diffuse;
	vec3 DiffuseColor = vLight.Color * vMaterial.Diffuse * max(dot(vNormalW, vLight.Direction), 0.0);
	vec3 ReflectDir = normalize(reflect(-vLight.Direction, _NormalW));
	vec3 SpecularColor = vLight.Color * vMaterial.Specular * pow(max(dot(vViewDir, ReflectDir), 0.0), vMaterial.Shinness);

	return AmbientColor + DiffuseColor + SpecularColor;
}

void main()
{
	vec3 ViewDirW = normalize(uViewPos - _PositionW);
	vec3 NormalW = normalize(_NormalW);

	SMaterial Material;
	Material.Diffuse = texture(uMaterialDiffuse1, _TexCoord).rgb;
	Material.Specular = texture(uMaterialSpecular1, _TexCoord).rgb;
	Material.Shinness = 32.0;

	vec3 FinalColor = vec3(0.0);
	FinalColor += computePhongShading4ParallelLight(_PositionW, NormalW, ViewDirW, SParallelLight(vec3(0.7), vec3(1.0, 1.0, 1.0)), Material);
	FinalColor += computePhongShading4ParallelLight(_PositionW, NormalW, ViewDirW, SParallelLight(vec3(0.7), vec3(-1.0, 1.0, -1.0)), Material);

	_outFragColor = vec4(FinalColor, 1.0);
}