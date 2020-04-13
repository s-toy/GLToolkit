#version 430

#expect WORK_GROUP_SIZE_X
#expect WORK_GROUP_SIZE_Y

layout(local_size_x = WORK_GROUP_SIZE_X, local_size_y = WORK_GROUP_SIZE_Y, local_size_z = 1) in;

struct ListNode
{
	uint packedColor;
	uint depthAndCoverage;
	uint next;
};

layout(r32ui) uniform uimage2D uListHeadPtrTex;

layout(binding = 0, std430) buffer linkedLists
{
	ListNode nodes[];
};

void insertionSort(uint headIndex)
{
	uint p = nodes[headIndex].next;

	while (p != 0)
	{
		uint prev = nodes[p].prev;
		uint curr = p;
		while (prev != 0)
		{
			ListNode prevNode = nodes[prev];
			ListNode currNode = nodes[curr];

			if (prevNode.depth <= currNode.depth) break;
			nodes[prev].depth = currNode.depth;
			nodes[curr].depth = prevNode.depth;

			curr = prev;
			prev = prevNode.prev;
		}
		p = nodes[p].next;
	}
}

void main()
{
	uint headIndex = imageLoad(uListHeadPtrTex, ivec2(gl_GlobalInvocationID.xy)).r;
	if (headIndex == 0) return;

	insertionSort(headIndex);
}