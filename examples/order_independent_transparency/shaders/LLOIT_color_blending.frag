#version 430 core

struct ListNode
{
	uint packedColor;
	uint transmittance;
	uint depth;
	uint next;
};

struct NodeData
{
	uint packedColor;
	uint transmittance;
	float depth;
};

#define MAX_FRAGMENTS 16

layout(binding = 0, r32ui) coherent uniform uimage2D uListHeadPtrTex;
layout(binding = 0, std430) buffer linkedList { ListNode nodes[]; };

layout(location = 0) out vec4 _outFragColor;

vec4 unpackColor(uint color)
{
	vec4 c;
	c.r = float((color >> 24) & 0x000000ff) / 255.0f;
	c.g = float((color >> 16) & 0x000000ff) / 255.0f;
	c.b = float((color >> 8) & 0x000000ff) / 255.0f;
	c.a = float(color & 0x000000ff) / 255.0f;
	return c;
}

void insertionSort(inout NodeData sortedFragments[MAX_FRAGMENTS], int size)
{
	for (int k = 1; k < size; k++)
	{
		int j = k;
		NodeData t = sortedFragments[k];
			
		while (sortedFragments[j - 1].depth < t.depth)
		{
			sortedFragments[j] = sortedFragments[j - 1];
			j--;
			if (j <= 0) { break; }
		}

		if (j != k) { sortedFragments[j] = t; }
	}
}

void main()
{
	uint index = imageLoad(uListHeadPtrTex, ivec2(gl_FragCoord.xy)).x;
	if (index == 0) { _outFragColor = vec4(0.0, 0.0, 0.0, 1.0); return; }

	NodeData fragments[MAX_FRAGMENTS];
	int counter = 0;
	for (int i = 0; i < MAX_FRAGMENTS; i++)
	{
		fragments[i] = NodeData(0, 0, 0.0f);
		if (index != 0)
		{
			fragments[counter].packedColor = nodes[index].packedColor;
			fragments[counter].transmittance = nodes[index].transmittance;
			fragments[counter].depth = unpackHalf2x16(nodes[index].depth).x;
			counter++;
			index = nodes[index].next;
		}
	}

	insertionSort(fragments, counter);

	vec3 color = vec3(0.0);
	vec3 totalTransmittance = vec3(1.0);

	for (int i = 0; i < counter; i++)
	{
		vec4 c = unpackColor(fragments[i].packedColor);
		vec3 t = unpackColor(fragments[i].transmittance).rgb;
		totalTransmittance *= (1.0 - c.a + c.a * t);
		color = (1.0 - c.a) * color + c.a * (c.rgb + t * color);
	}
	
	_outFragColor = vec4(color, totalTransmittance);
}