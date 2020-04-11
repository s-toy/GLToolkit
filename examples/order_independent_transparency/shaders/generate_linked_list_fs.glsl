#version 430 core
#extension GL_ARB_fragment_shader_interlock : require

layout(r32ui) uniform uimage2D uListHeadPtrTex;
layout(binding = 0, offset = 0) uniform atomic_uint uListNodeCounter;

uniform int uMaxListNode;

uniform sampler2D	uMaterialDiffuse;
uniform sampler2D	uMaterialSpecular;

uniform vec3		uViewPos = vec3(0.0);

struct ListNode
{
	uint packedColor;
	uint depthAndCoverage;
	uint next;
};

layout(binding = 0, std430) buffer linkedLists
{
	ListNode nodes[];
};

void main()
{
	beginInvocationInterlockARB();

	uint nodeIndex = atomicCounterIncrement(uListNodeCounter) + 1u;
	if (nodeIndex >= uMaxListNode) discard;

	uint nextIndex = imageAtomicExchange(uListHeadPtrTex, ivec2(gl_FragCoord.xy), nodeIndex);
	nodes[nodeIndex] = ListNode(gl_FragCoord.z, 0, nextIndex);
	if (nextIndex != 0) nodes[nextIndex].prev = nodeIndex;

	endInvocationInterlockARB();
}