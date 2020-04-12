#version 430 core
#extension GL_ARB_fragment_shader_interlock : require

layout(binding = 0, r32ui) uniform uimage2D uListHeadPtrTex;
layout(binding = 1, offset = 0) uniform atomic_uint uListNodeCounter;

uniform int uMaxListNode;

uniform sampler2D	uMaterialDiffuse;
uniform sampler2D	uMaterialSpecular;

uniform vec3		uViewPos = vec3(0.0);

layout(location = 0) out vec3 _outFragColor;

struct ListNode
{
	uint packedColor;
	uint depthAndCoverage;
	uint next;
};

layout(binding = 2, std430) buffer linkedLists
{
	ListNode nodes[];
};

void main()
{
	beginInvocationInterlockARB();

	uint nodeIndex = atomicCounterIncrement(uListNodeCounter) + 1u;

	if (nodeIndex < uMaxListNode)
	{
		uint nextIndex = imageAtomicExchange(uListHeadPtrTex, ivec2(gl_FragCoord.xy), nodeIndex);
		nodes[nodeIndex] = ListNode(0, 0, nextIndex);
	}

	endInvocationInterlockARB();

	_outFragColor = vec3(float(nodeIndex)/100000.0f);
}