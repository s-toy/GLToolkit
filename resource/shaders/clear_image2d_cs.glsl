#version 460 core

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

//layout(binding = 0, r32ui) uniform uimage2D uListHeadPtrTex;
layout(binding = 1, rgba32f) uniform image2D uListHeadPtrTex;

void main()
{
	//imageStore(uListHeadPtrTex, ivec2(gl_GlobalInvocationID.xy), ivec4(0));
	imageStore(uListHeadPtrTex, ivec2(gl_GlobalInvocationID.xy), vec4(0.0));
}