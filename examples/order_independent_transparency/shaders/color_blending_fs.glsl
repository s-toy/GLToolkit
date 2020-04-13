#version 430 core

struct ListNode
{
	uint packedColor;
	uint depthAndCoverage;
	uint next;
};

struct NodeData
{
	uint packedColor;
	float depth;
};

#define MAX_FRAGMENTS 16

#define BLENDING_STRATEGY_NO_SORTING		0
#define BLENDING_STRATEGY_PERFECT_SORTING	1
#define BLENDING_STRATEGY_OIT_16			2		

uniform int uBlendingStrategy = BLENDING_STRATEGY_PERFECT_SORTING;

layout(binding = 0, r32ui) coherent uniform uimage2D uListHeadPtrTex;
layout(binding = 2, std430) buffer linkedList { ListNode nodes[]; };

layout(location = 0) out vec4 _outFragColor;

vec4 unpackColor(uint color)
{
	vec4 c;
	c.r = float((color >> 24) & 0x000000ff) / 255.0f;
	c.g = float((color >> 16) & 0x000000ff) / 255.0f;
	c.b = float((color >> 8) & 0x000000ff) / 255.0f;
	c.a = float(color & 0x000000ff) / 255.0f;
	return clamp(c, 0.0f, 1.0f);
}

void insertionSort(inout NodeData sortedFragments[MAX_FRAGMENTS])
{
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
	uint index = imageLoad(uListHeadPtrTex, ivec2(gl_FragCoord.xy)).x;
	if (index == 0) { _outFragColor = vec4(0.0, 0.0, 0.0, 1.0); return; }

	vec3 color = vec3(0);
	float alpha = 1.0f;

	NodeData fragments[MAX_FRAGMENTS];
	int counter = 0;
	for (int i = 0; i < MAX_FRAGMENTS; i++)
	{
		fragments[i] = NodeData(0, 0.0f);
		if (index != 0)
		{
			fragments[counter].packedColor = nodes[index].packedColor;
			fragments[counter].depth = unpackHalf2x16(nodes[index].depthAndCoverage).x;
			counter++;
			index = nodes[index].next;
		}
	}

	if (uBlendingStrategy == BLENDING_STRATEGY_PERFECT_SORTING)
	{
		insertionSort(fragments);
	}

	if (uBlendingStrategy == BLENDING_STRATEGY_NO_SORTING || uBlendingStrategy == BLENDING_STRATEGY_PERFECT_SORTING)
	{
		for (int i = 0; i < counter; i++)
		{
			vec4 c = unpackColor(fragments[i].packedColor);
			alpha *= (1.0 - c.a);
			color = mix(color, c.rgb, c.a);
		}
	}
	else if (uBlendingStrategy == BLENDING_STRATEGY_OIT_16)
	{
		float totalWeight = 0.0;
		for (int i = 0; i < counter; i++)
		{
			vec4 c = unpackColor(fragments[i].packedColor);
			alpha *= (1.0 - c.a);

			float weight = clamp(pow(10.0*(1.0-0.99*gl_FragCoord.z)*c.a, 3.0), 0.01, 30.0);
			color += c.rgb * c.a * weight;
			totalWeight += c.a * weight;
		}

		color = (1.0 - alpha) * color / totalWeight;
	}

	_outFragColor = vec4(color, alpha);
}