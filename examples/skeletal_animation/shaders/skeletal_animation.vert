#version 430 core

const int MAX_BONES = 100;

uniform mat4 uModelMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;
uniform mat4 uBonesMatrix[MAX_BONES];
uniform bool uHasBones = false;

layout(location = 0) in vec3 _inVertexPosition;
layout(location = 1) in vec3 _inVertexNormal;
layout(location = 2) in vec2 _inVertexTexCoord;
layout(location = 3) in ivec4 _inBoneIDs;
layout(location = 4) in vec4  _inBoneWeights;

layout(location = 0) out vec3 _outPositionW;
layout(location = 1) out vec3 _outNormalW;
layout(location = 2) out vec2 _outTexCoord;

void boneTransform(inout vec4 pos, inout vec4 normal)
{
	mat4 BoneTransform = uBonesMatrix[_inBoneIDs[0]] * _inBoneWeights[0];
	BoneTransform += uBonesMatrix[_inBoneIDs[1]] * _inBoneWeights[1];
	BoneTransform += uBonesMatrix[_inBoneIDs[2]] * _inBoneWeights[2];
	BoneTransform += uBonesMatrix[_inBoneIDs[3]] * _inBoneWeights[3];
	pos = BoneTransform * pos;
	normal = BoneTransform * normal;
}

void main()
{
	vec4 pos = vec4(_inVertexPosition, 1.0);
	vec4 normal = vec4(_inVertexNormal, 0.0);

	if (uHasBones) boneTransform(pos, normal);

	_outPositionW.xyz = vec3(uModelMatrix * pos);
	_outNormalW = mat3(transpose(inverse(uModelMatrix))) * normal.xyz;
	_outTexCoord = _inVertexTexCoord;
	gl_Position = uProjectionMatrix * uViewMatrix * vec4(_outPositionW.xyz, 1.0);
}