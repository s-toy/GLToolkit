#version 430 core

uniform sampler2D uOpaqueColorTex;

struct ListNode
{
	uint packedColor;
	uint depthAndCoverage;
	uint next;
};

layout(binding = 0, r32ui) uniform uimage2D uListHeadPtrTex;

layout(binding = 2, std430) buffer linkedLists
{
	ListNode nodes[];
};

layout(location = 0) in vec2 _inTexCoord;

vec4 unpackColor(uint color)
{
	vec4 unpack;
	unpack.r = float((color & uint(0xff000000)) >> 24) / 255.0;
	unpack.g = float((color & uint(0x00ff0000)) >> 16) / 255.0;
	unpack.b = float((color & uint(0x0000ff00)) >> 8)  / 255.0;
	unpack.a = float((color & uint(0x000000ff)) >> 0)  / 255.0;
	return unpack;
}

void main()
{
	vec3 color = texture(uOpaqueColorTex, _inTexCoord).rgb;

	uint headIndex = imageLoad(uListHeadPtrTex, ivec2(gl_FragCoord.xy)).r;
	if (headIndex != 0)
	{
		ListNode node = nodes[headIndex];
		color = unpackColor(node.packedColor).rgb;
	}

	gl_FragColor = vec4(color, 1.0);
}