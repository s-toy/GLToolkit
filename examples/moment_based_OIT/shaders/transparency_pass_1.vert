#version 430 core

uniform mat4 uModelMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;

layout(location = 0) in vec3 _inVertexPosition;
layout(location = 1) in vec3 _inVertexNormal;
layout(location = 2) in vec2 _inVertexTexCoord;

void main()
{
	gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * vec4(_inVertexPosition, 1.0);
}