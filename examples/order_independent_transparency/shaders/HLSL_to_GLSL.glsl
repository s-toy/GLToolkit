#define float2		vec2
#define float3		vec3
#define float4		vec4
#define float2x2	mat2
#define float2x3	mat2x3
#define float2x4	mat2x4
#define float3x2	mat3x2
#define float3x3	mat3
#define float3x4	mat3x4
#define float4x2	mat4x2
#define float4x3	mat4x3
#define float4x4	mat4
#define atan2		atan
#define fmod		mod
#define lerp		mix
#define frac		fract
#define rsqrt		inversesqrt

#define mad(m, a, b)		((m) * (a) + (b))
#define mul(a, b)			((a) * (b))
#define saturate(s)			clamp((s), 0.0, 1.0)

void sincos(float x, out float s, out float c) { s = sin(x); c = cos(x); }