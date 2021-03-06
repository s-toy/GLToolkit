#version 460 core
#extension GL_NV_fragment_shader_interlock : require

#include "common.glsl"

uniform sampler2D   uOpaqueDepthTex;
uniform float		uNearPlane;
uniform float		uFarPlane;


layout(location = 0) in float _inFragDepth;

layout(location = 0) out float _outEmpty;

layout(binding = 5, rg16f)	 uniform image2D		uSurfaceZImage;

void main()
{
	float opaqueDepth = texelFetch(uOpaqueDepthTex, ivec2(gl_FragCoord.xy), 0).r;
	if (opaqueDepth != 0.0 && gl_FragCoord.z > opaqueDepth) discard;

	beginInvocationInterlockNV();

	float depth = _linearizeDepth(gl_FragCoord.z, uNearPlane, uFarPlane);

	vec2 surfaceZ = imageLoad(uSurfaceZImage, ivec2(gl_FragCoord.xy)).xy;
	imageStore(uSurfaceZImage, ivec2(gl_FragCoord.xy), vec4(min(depth, surfaceZ.x), max(depth, surfaceZ.y), 0, 0));

	endInvocationInterlockNV();

	_outEmpty = 0;
}