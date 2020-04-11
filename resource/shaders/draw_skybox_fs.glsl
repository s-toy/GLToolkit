#version 430 core

layout(location = 0) in vec3 _inTexCoords;

layout(location = 0) out vec3 _outFragColor;

uniform samplerCube uSkyboxTex;

void main()
{
    _outFragColor = texture(uSkyboxTex, _inTexCoords).rgb;
}