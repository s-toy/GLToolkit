#version 460 core

layout(location = 0) in vec3 _inTexCoords;

layout(location = 0) out vec3 _outFragColor;
layout(location = 1) out float _outFragDepth;

uniform samplerCube uSkyboxTex;

void main()
{
    _outFragColor = texture(uSkyboxTex, _inTexCoords).rgb;
    _outFragDepth = 0.0;
}