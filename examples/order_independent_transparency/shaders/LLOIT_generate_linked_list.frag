#version 460 core
#extension GL_ARB_fragment_shader_interlock : require
#include "common.glsl"
#include "compute_phong_shading.glsl"

struct ListNode
{
	uint packedColor;
	uint transmittance;
	uint depth;
	uint next;
};

layout(binding = 0, r32ui) uniform uimage2D uListHeadPtrTex;
layout(binding = 0, offset = 0) uniform atomic_uint uListNodeCounter;
layout(binding = 0, std430) buffer linkedLists { ListNode nodes[]; };

uniform int			uMaxListNode;
uniform sampler2D	uMaterialDiffuse;
uniform sampler2D	uMaterialSpecular;
uniform sampler2D   uOpaqueDepthTex;
uniform vec3		uViewPos = vec3(0.0);
uniform float		uNearPlane;
uniform float		uFarPlane;

uniform vec3	uDiffuseColor;
uniform vec3	uTransmittance;
uniform float	uCoverage;

layout(location = 0) in vec3 _inPositionW;
layout(location = 1) in vec3 _inNormalW;
layout(location = 2) in vec2 _inTexCoord;

layout(location = 0) out vec4 _outFragColor;

uint packColor(vec4 color)
{
	uvec4 bytes = uvec4(color * 255.0);
	uint pack = (bytes.r << 24) | (bytes.g << 16) | (bytes.b << 8) | (bytes.a);
	return pack;
}

#include "compute_reflection_color.glsl"

void main()
{
	float depth = texelFetch(uOpaqueDepthTex, ivec2(gl_FragCoord.xy), 0).r;
	if (depth != 0.0 && gl_FragCoord.z > depth) discard;

	vec3 color = computeReflectColor();
	uint packedColor = packColor(vec4(color, uCoverage));
	uint transmittance = packColor(vec4(uTransmittance, 0.0));

	beginInvocationInterlockARB();

	uint nodeIndex = atomicCounterIncrement(uListNodeCounter) + 1u;

	if (nodeIndex < uMaxListNode)
	{
		uint nextIndex = imageAtomicExchange(uListHeadPtrTex, ivec2(gl_FragCoord.xy), nodeIndex);
		uint currentDepth = packHalf2x16(vec2(_linearizeDepth(gl_FragCoord.z, uNearPlane, uFarPlane), 0));
		nodes[nodeIndex] = ListNode(packedColor, transmittance, currentDepth, nextIndex);
	}

	endInvocationInterlockARB();

	//_outFragColor = vec4(color, 1.0);
}