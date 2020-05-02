#version 430 core
#include "skeletal_animation_input.glsl"

layout(location = 0) out vec3 _outPositionW;
layout(location = 1) out vec3 _outNormalW;
layout(location = 2) out vec2 _outTexCoord;

void main()
{
	vec4 pos = vec4(_inVertexPosition, 1.0);
	vec4 normal = vec4(_inVertexNormal, 0.0);

	if (uHasBones) boneTransform(pos, normal);

	_outPositionW = vec3(uModelMatrix * pos);
	_outNormalW = mat3(transpose(inverse(uModelMatrix))) * normal.xyz;
	_outTexCoord = _inVertexTexCoord;
	gl_Position = uProjectionMatrix * uViewMatrix * vec4(_outPositionW.xyz, 1.0);
}