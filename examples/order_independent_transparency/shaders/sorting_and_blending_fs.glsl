#version 430 core

out vec4 outputColor;

layout(binding = 0, r32ui) coherent uniform uimage2D headBuffer;

struct ListNode
{
	uint packedColor;
	uint depthAndCoverage;
	uint next;
};
layout(binding = 2, std430) buffer fragmentsList
{
	ListNode fragments[];
};

struct NodeData
{
	uint packedColor;
	float depth;
};

const int MAX_FRAGMENTS = 16;

vec4 unpackColor(uint color)
{
	vec4 c;
	c.r = float((color >> 24) & 0x000000ff) / 255.0f;
	c.g = float((color >> 16) & 0x000000ff) / 255.0f;
	c.b = float((color >> 8) & 0x000000ff) / 255.0f;
	c.a = float(color & 0x000000ff) / 255.0f;
	return clamp(c, 0.0f, 1.0f);
}

void insertionSort(uint startIndex, inout NodeData sortedFragments[MAX_FRAGMENTS], out int counter)
{
	counter = 0;
	uint index = startIndex;
	for (int i = 0; i < MAX_FRAGMENTS; i++)
	{
		if (index != 0)
		{
			sortedFragments[counter].packedColor = fragments[index].packedColor;
			sortedFragments[counter].depth = unpackHalf2x16(fragments[index].depthAndCoverage).x;
			counter++;
			index = fragments[index].next;
		}
	}

	for (int k = 1; k < MAX_FRAGMENTS; k++)
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
	ivec2 upos = ivec2(gl_FragCoord.xy);
	uint index = imageLoad(headBuffer, upos).x;
	if (index == 0) { outputColor = vec4(0.0, 0.0, 0.0, 1.0); return; }

	vec3 color = vec3(0);
	float alpha = 1.0f;

	NodeData sortedFragments[MAX_FRAGMENTS];
	for (int i = 0; i < MAX_FRAGMENTS; i++)
	{
		sortedFragments[i] = NodeData(0, 0.0f);
	}

	int counter;
	insertionSort(index, sortedFragments, counter);
	for (int i = 0; i < MAX_FRAGMENTS; i++)
	{
		if (i < counter)
		{
			vec4 c = unpackColor(sortedFragments[i].packedColor);
			alpha *= (1.0 - c.a);
			color = mix(color, c.rgb, c.a);
		}
	}
	
	outputColor = vec4(color, alpha);
}