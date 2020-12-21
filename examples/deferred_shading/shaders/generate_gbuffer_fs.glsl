#version 460 core

uniform sampler2D uMaterialDiffuseTex;
uniform sampler2D uMaterialSpecularTex;

layout(location = 0) in vec3 _inPositionW;
layout(location = 1) in vec3 _inNormalW;
layout(location = 2) in vec2 _inTexCoord;

layout (location = 0) out vec3 _outPositionW;
layout (location = 1) out vec3 _outNormalW;
layout (location = 2) out vec3 _outDiffuseColor;
layout (location = 3) out vec3 _outSpecularColor;

void main()
{
	_outPositionW = _inPositionW;
	_outNormalW = _inNormalW;
	_outDiffuseColor = texture(uMaterialDiffuseTex, _inTexCoord).rgb;
	_outSpecularColor = texture(uMaterialSpecularTex, _inTexCoord).rgb;
}