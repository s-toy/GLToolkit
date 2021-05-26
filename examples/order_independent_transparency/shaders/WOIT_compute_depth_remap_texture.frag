#version 460 core
#include "global_macro.h"

uniform sampler2D uOpaqueColorTex;
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

vec2 intersectSphere( in vec3 rayOrigin, in vec3 rayDir, in vec3 ce, float ra )
{
    vec3 oc = rayOrigin - ce;
    float b = dot( oc, rayDir );
    float c = dot( oc, oc ) - ra*ra;
    float h = b*b - c;
    if( h<0.0 ) return vec2(-1.0); // no intersection
    h = sqrt( h );
    return vec2( -b-h, -b+h );
}

vec3 getRayFromScreenSpace(vec2 pos)
{
	float halfScreenWidth = 0.5 * float(DEPTH_REMAP_TEX_WIDTH);
	float halfScreenHeight = 0.5 * float(DEPTH_REMAP_TEX_HEIGHT);

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
	vec3 rayDir = getRayFromScreenSpace(vec2(gl_FragCoord.x, DEPTH_REMAP_TEX_HEIGHT - gl_FragCoord.y - 1));

	float tmin = 1e6, tmax = -1e6;

	for (int i = 0; i < uAABBNum; ++i)
	{
		//vec2 hitInfo = intersectAABB(rayOri, rayDir, minAABBVertices[i], maxAABBVertices[i]);
		vec2 hitInfo = intersectSphere(rayOri, rayDir, 0.5 * (minAABBVertices[i] + maxAABBVertices[i]), 0.5 * length(maxAABBVertices[i] - minAABBVertices[i]));

		if (hitInfo.x < hitInfo.y)
		{
			tmin = min(tmin, hitInfo.x);
			tmax = max(tmax, hitInfo.y);
		}
	}

	if (tmin < tmax)
	{
		vec3 minPosWS = rayOri + rayDir * tmin;
		vec3 maxPosWS = rayOri + rayDir * tmax;

		vec4 minPosVS = uViewMatrix * vec4(minPosWS, 1);
		vec4 maxPosVS = uViewMatrix * vec4(maxPosWS, 1);

		float minZ = (-minPosVS.z - NEAR_PLANE) / (FAR_PLANE - NEAR_PLANE);
		float maxZ = (-maxPosVS.z - NEAR_PLANE) / (FAR_PLANE - NEAR_PLANE);
		_outMinMaxDepth = vec2(minZ, maxZ);
	}
	else
	{
		_outMinMaxDepth = vec2(0);
	}
}