#version 460 core
#include "common.glsl"

uniform sampler2D uOpaqueColorTex;
uniform int uScreenWidth;
uniform int uScreenHeight;
uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;
uniform vec3 uViewPos;

layout (location = 0) out vec2 _outMinMaxDepth;

#define MAX_NUM_AABB 32

uniform int uAABBNum;
uniform vec3 minAABBVertices[MAX_NUM_AABB];
uniform vec3 maxAABBVertices[MAX_NUM_AABB];

vec2 intersectAABB(vec3 rayOrigin, vec3 rayDir, vec3 boxMin, vec3 boxMax) {
    vec3 tMin = (boxMin - rayOrigin) / rayDir;
    vec3 tMax = (boxMax - rayOrigin) / rayDir;
    vec3 t1 = min(tMin, tMax);
    vec3 t2 = max(tMin, tMax);
    float tNear = max(max(t1.x, t1.y), t1.z);
    float tFar = min(min(t2.x, t2.y), t2.z);
    return vec2(tNear, tFar);
};

vec3 getRayFromScreenSpace(vec2 pos)
{
	float halfScreenWidth = 0.5 * float(uScreenWidth);
	float halfScreenHeight = 0.5 * float(uScreenHeight);

    mat4 invMat= inverse(uProjectionMatrix*uViewMatrix);
    vec4 near = vec4((pos.x - halfScreenWidth) / halfScreenWidth, -1*(pos.y - halfScreenHeight) / halfScreenHeight, -1, 1.0);
    vec4 far = vec4((pos.x - halfScreenWidth) / halfScreenWidth, -1*(pos.y - halfScreenHeight) / halfScreenHeight, 1, 1.0);
    vec4 nearResult = invMat*near;
    vec4 farResult = invMat*far;
    nearResult /= nearResult.w;
    farResult /= farResult.w;
    vec3 dir = vec3(farResult - nearResult );
    return normalize(dir);
}

void main()
{
	vec3 rayOri = uViewPos;
	vec3 rayDir = getRayFromScreenSpace(vec2(gl_FragCoord.x, uScreenHeight - gl_FragCoord.y - 1));

	float hit = 0; 

	for (int i = 0; i < uAABBNum; ++i)
	{
		vec2 hitInfo = intersectAABB(rayOri, rayDir, minAABBVertices[i], maxAABBVertices[i]);
		float tnear = hitInfo.x;
		float tfar = hitInfo.y;

		if (tnear < tfar)
		{
			vec3 nearPos = rayOri + tnear * rayDir;
			vec3 farPos = rayOri + tfar * rayDir;
			hit = 1;
		}
	}

	_outMinMaxDepth = vec2(hit);
}