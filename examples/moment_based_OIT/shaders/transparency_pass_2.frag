#version 430 core

struct SParallelLight { vec3 Color; vec3 Direction; };

struct SMaterial { vec3 Diffuse; vec3 Specular; float Shinness; };

uniform sampler2D	uMaterialDiffuse;
uniform sampler2D	uMaterialSpecular;
uniform sampler2D   uOpaqueDepthTex;
uniform vec3		uViewPos = vec3(0.0);

uniform vec3	uDiffuseColor;
uniform float	uCoverage;

layout(location = 0) in vec3 _inPositionW;
layout(location = 1) in vec3 _inNormalW;
layout(location = 2) in vec2 _inTexCoord;

layout(location = 0) out vec4 _outTransparencyColor;

vec3 computePhongShading4ParallelLight(vec3 vPositionW, vec3 vNormalW, vec3 vViewDir, SParallelLight vLight, SMaterial vMaterial)
{
	vec3 AmbientColor = 0.2 * vLight.Color * vMaterial.Diffuse;
	vec3 DiffuseColor = vLight.Color * vMaterial.Diffuse * max(dot(vNormalW, vLight.Direction), 0.0);
	vec3 ReflectDir = normalize(reflect(-vLight.Direction, _inNormalW));
	vec3 SpecularColor = vLight.Color * vMaterial.Specular * pow(max(dot(vViewDir, ReflectDir), 0.0), vMaterial.Shinness);

	return AmbientColor + DiffuseColor /*+ SpecularColor*/;
}

vec3 computeReflectColor()
{
	vec3 ViewDirW = normalize(uViewPos - _inPositionW);
	vec3 NormalW = normalize(_inNormalW);

	SMaterial Material;
	Material.Diffuse = uDiffuseColor; // texture(uMaterialDiffuse, _inTexCoord).rgb;
	Material.Specular = vec3(0.0); // texture(uMaterialSpecular, _inTexCoord).rgb;
	Material.Shinness = 32.0;

	vec3 color = vec3(0.0);
	color += computePhongShading4ParallelLight(_inPositionW, NormalW, ViewDirW, SParallelLight(vec3(0.7), vec3(1.0, 1.0, 1.0)), Material);
	color += computePhongShading4ParallelLight(_inPositionW, NormalW, ViewDirW, SParallelLight(vec3(0.7), vec3(-1.0, 1.0, -1.0)), Material);

	return color;
}

void main()
{
	float depth = texelFetch(uOpaqueDepthTex, ivec2(gl_FragCoord.xy), 0).r;
	if (depth != 0.0 && gl_FragCoord.z > depth) discard;

	vec3  color = computeReflectColor();
	float transmittance = pow(10.0 * (1.0 - 0.99 * gl_FragCoord.z) * uCoverage, 3.0);
	transmittance = clamp(transmittance, 0.01, 30.0);

	_outTransparencyColor.rgb = transmittance * uCoverage * color;
	_outTransparencyColor.a = transmittance * uCoverage;
}