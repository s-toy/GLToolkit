#version 460 core

uniform mat4 uModelMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;

layout(location = 0) in vec3 _inVertexPosition;
layout(location = 1) in vec3 _inVertexNormal;
layout(location = 2) in vec2 _inVertexTexCoord;

out vec3 _PositionW;
out vec3 _NormalW;
out vec2 _TexCoord;

void main()
{
	_PositionW = vec3(uModelMatrix * vec4(_inVertexPosition, 1.0));
	_NormalW = mat3(transpose(inverse(uModelMatrix))) * _inVertexNormal;
	_TexCoord = _inVertexTexCoord;
	gl_Position = uProjectionMatrix * uViewMatrix * vec4(_PositionW, 1.0);
}