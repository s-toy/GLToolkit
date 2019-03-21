#version 430 core

uniform mat4 uModelMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;

layout (location = 0) in vec3 _inVertexPosition;
layout (location = 1) in vec3 _inVertexNormal;
layout (location = 2) in vec2 _inVertexTexCoord;

out vec3 _PositionE;
out vec3 _NormalE;
out vec2 _TexCoordV;

void main()
{
	_PositionE = (uModelMatrix * vec4(_inVertexPosition, 1.0)).xyz;
	_NormalE = mat3(transpose(inverse(uModelMatrix))) * _inVertexNormal;
	_TexCoordV = _inVertexTexCoord;
	gl_Position = uProjectionMatrix * uViewMatrix * vec4(_PositionE, 1.0);
}