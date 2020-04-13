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

#define BLENDING_STRATEGY_NO_SORTING		0
#define BLENDING_STRATEGY_PERFECT_SORTING	1
#define BLENDING_STRATEGY_OIT_16			2		

uniform int uBlendingStrategy = BLENDING_STRATEGY_PERFECT_SORTING;

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
	return clamp(c, 0.0f, 1.0f);
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

	if (uBlendingStrategy == BLENDING_STRATEGY_PERFECT_SORTING)
	{
		insertionSort(fragments, counter);
	}

	vec3 color = vec3(0.0);
	vec3 beta = vec3(1.0);

	if (uBlendingStrategy == BLENDING_STRATEGY_NO_SORTING || uBlendingStrategy == BLENDING_STRATEGY_PERFECT_SORTING)
	{
		for (int i = 0; i < counter; i++)
		{
			vec4 c = unpackColor(fragments[i].packedColor);
			vec3 t = unpackColor(fragments[i].transmittance).rgb;
			beta *= (1.0 - c.a + c.a * t);
			color = (1.0 - c.a) * color + c.a * (c.rgb + t * color);
		}
	}
	else if (uBlendingStrategy == BLENDING_STRATEGY_OIT_16)
	{
		float totalWeight = 0.0;
		for (int i = 0; i < counter; i++)
		{
			vec4 c = unpackColor(fragments[i].packedColor);
			vec3 t = unpackColor(fragments[i].transmittance).rgb;
			float t_average = (t.r + t.g + t.b) / 3.0;
			beta *= (1.0 - c.a + c.a * t);

			float weight = pow(10.0 * (1.0 - 0.99 * gl_FragCoord.z) * c.a * (1.0 - t_average), 3.0);
			weight = clamp(weight, 0.01, 30.0);
			color +=  c.a * weight * c.rgb;
			totalWeight += c.a * weight * (1.0 - t_average);
		}

		color = (vec3(1.0) - beta) * color / totalWeight;
	}

	_outFragColor = vec4(color, beta);
}