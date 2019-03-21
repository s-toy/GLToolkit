#version 430 core

uniform vec3 uMaterialAmbient = vec3(0.6, 0.6, 0.6);
uniform vec3 uMaterialDiffuse = vec3(0.6, 0.4, 0.4);
uniform vec3 uMaterialSpecular = vec3(0.01, 0.01, 0.01);
uniform float uMaterialShinness = 2.0;

uniform vec3 uLightDirectionE = vec3(-1, 1, -1);
uniform vec3 uLightAmbient = vec3(0.4, 0.4, 0.4);
uniform vec3 uLightDiffuse = vec3(0.8, 0.8, 0.8);
uniform vec3 uLightSpecular = vec3(0.01, 0.01, 0.01);

uniform vec3 uViewPos;

in vec3 _PositionE;
in vec3 _NormalE;
in vec2 _TexCoordV;

out vec4 _outFragColor;

vec4 computePhongShading4ParallelLight(vec3 vPositionE, vec3 vNormalE, vec3 vViewDir)
{
	vec4 Result;

	vec3 Ambient = uLightAmbient * uMaterialAmbient;
	vec3 Diffuse = uLightDiffuse * uMaterialDiffuse * max(dot(vNormalE, uLightDirectionE), 0.0);
	vec3 ReflectDir = reflect(-uLightDirectionE, _NormalE);
	float Spec = pow(max(dot(vViewDir, ReflectDir), 0.0), uMaterialShinness);
	vec3 Specular = uLightSpecular * uMaterialSpecular * Spec;

	Result = vec4(Ambient + Diffuse, 1.0f);

	return Result;
}

void main()
{ 
	vec3 ViewDir = normalize(uViewPos - _PositionE);
	_outFragColor = computePhongShading4ParallelLight(_PositionE, normalize(_NormalE), ViewDir);
}