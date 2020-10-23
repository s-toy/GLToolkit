#version 460 core
#include "skeletal_animation_input.glsl"

layout(location = 0) out float _outFragDepth;

void main()
{
	vec4 pos = vec4(_inVertexPosition, 1.0);
	vec4 normal = vec4(_inVertexNormal, 0.0);
	if (uHasBones) boneTransform(pos, normal);

	gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * pos;
	_outFragDepth = gl_Position.z / gl_Position.w;
}