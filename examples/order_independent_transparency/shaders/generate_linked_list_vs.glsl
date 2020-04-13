#version 430 core

uniform mat4 uModelMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;

layout(location = 0) in vec3 _inVertexPosition;
layout(location = 1) in vec3 _inVertexNormal;
layout(location = 2) in vec2 _inVertexTexCoord;

layout(location = 0) out vec4 _outPositionW;
layout(location = 1) out vec3 _outNormalW;
layout(location = 2) out vec2 _outTexCoord;

void main()
{
	_outPositionW.xyz = vec3(uModelMatrix * vec4(_inVertexPosition, 1.0));
	_outNormalW = mat3(transpose(inverse(uModelMatrix))) * _inVertexNormal;
	_outTexCoord = _inVertexTexCoord;
	gl_Position = uProjectionMatrix * uViewMatrix * vec4(_outPositionW.xyz, 1.0);
	_outPositionW.w = gl_Position.z;
}