#version 430 core
#extension GL_ARB_fragment_shader_interlock : require

struct ListNode
{
	uint packedColor;
	uint depthAndCoverage;
	uint next;
};

struct SParallelLight { vec3 Color; vec3 Direction; };

struct SMaterial { vec3 Diffuse; vec3 Specular; float Shinness; };

layout(binding = 0, r32ui) uniform uimage2D uListHeadPtrTex;

layout(binding = 1, offset = 0) uniform atomic_uint uListNodeCounter;

layout(binding = 2, std430) buffer linkedLists
{
	ListNode nodes[];
};

uniform int			uMaxListNode;
uniform sampler2D	uMaterialDiffuse;
uniform sampler2D	uMaterialSpecular;
uniform sampler2D   uOpaqueDepthTex;
uniform vec3		uViewPos = vec3(0.0);

layout(location = 0) in vec4 _inPositionW;
layout(location = 1) in vec3 _inNormalW;
layout(location = 2) in vec2 _inTexCoord;

layout(location = 0) out vec4 _outFragColor;

vec3 computePhongShading4ParallelLight(vec3 vPositionW, vec3 vNormalW, vec3 vViewDir, SParallelLight vLight, SMaterial vMaterial)
{
	vec3 AmbientColor = 0.2 * vLight.Color * vMaterial.Diffuse;
	vec3 DiffuseColor = vLight.Color * vMaterial.Diffuse * max(dot(vNormalW, vLight.Direction), 0.0);
	vec3 ReflectDir = normalize(reflect(-vLight.Direction, _inNormalW));
	vec3 SpecularColor = vLight.Color * vMaterial.Specular * pow(max(dot(vViewDir, ReflectDir), 0.0), vMaterial.Shinness);

	return AmbientColor + DiffuseColor + SpecularColor;
}

vec3 computeColor()
{
	vec3 ViewDirW = normalize(uViewPos - _inPositionW.xyz);
	vec3 NormalW = normalize(_inNormalW);

	SMaterial Material;
	Material.Diffuse = texture(uMaterialDiffuse, _inTexCoord).rgb;
	Material.Specular = texture(uMaterialSpecular, _inTexCoord).rgb;
	Material.Shinness = 32.0;

	vec3 color = vec3(0.0);
	color += computePhongShading4ParallelLight(_inPositionW.xyz, NormalW, ViewDirW, SParallelLight(vec3(0.7), vec3(1.0, 1.0, 1.0)), Material);
	color += computePhongShading4ParallelLight(_inPositionW.xyz, NormalW, ViewDirW, SParallelLight(vec3(0.7), vec3(-1.0, 1.0, -1.0)), Material);

	return color;
}

uint packColor(vec4 color)
{
	uvec4 bytes = uvec4(color * 255.0);
	uint pack = (bytes.r << 24) | (bytes.g << 16) | (bytes.b << 8) | (bytes.a);
	return pack;
}

void main()
{
	float depth = texelFetch(uOpaqueDepthTex, ivec2(gl_FragCoord.xy), 0).r;
	if (depth != 0.0 && gl_FragCoord.z > depth) discard;

	vec3 color = computeColor();
	//color = vec3(0.5);
	uint packedColor = packColor(vec4(color, 0.5));

	beginInvocationInterlockARB();

	uint nodeIndex = atomicCounterIncrement(uListNodeCounter) + 1u;

	if (nodeIndex < uMaxListNode)
	{
		uint nextIndex = imageAtomicExchange(uListHeadPtrTex, ivec2(gl_FragCoord.xy), nodeIndex);
		uint currentDepth = packHalf2x16(vec2(_inPositionW.w, 0));
		nodes[nodeIndex] = ListNode(packedColor, currentDepth, nextIndex);
	}

	endInvocationInterlockARB();

	//_outFragColor = vec4(color, 1.0);
}