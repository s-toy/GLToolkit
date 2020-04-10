#version 430 core

uniform float	iTime;
uniform vec2	iResolution;
uniform vec4	iMouse;

layout(location = 0) in vec2 _fragCoord;

layout(location = 0) out vec4 _fragColor;

void mainImage(out vec4 fragColor, in vec2 fragCoord);

void main()
{
	mainImage(_fragColor, _fragCoord * iResolution.xy);
}

//------------------------------------------------------------------------
//----------------SHADERTOY CODES-----------------------------------------

#define AA 2

struct Material
{
    vec3 	diffuseAlbedo;
    vec3 	specularAlbedo;
    float 	specularPower;
};

Material materials[] = Material[] 
(
    Material(vec3(0.0, 0.2, 0.2), vec3(0.3), 8.0),
    Material(vec3(0.2, 0.2, 0.0), vec3(0.3), 8.0),
    Material(vec3(0.2, 0.0, 0.2), vec3(0.3), 8.0),
    Material(vec3(0.2, 0.2, 0.2), vec3(0.2), 8.0)
);

vec2 opU(vec2 a, vec2 b) { return a.x < b.x ? a : b; }

//distance functions from http://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm
float sdOctahedron( vec3 p, float s)
{
  p = abs(p);
  return (p.x+p.y+p.z-s)*0.57735027;
}

float sdTorus( vec3 p, vec2 t )
{
  vec2 q = vec2(length(p.xz)-t.x,p.y);
  return length(q)-t.y;
}

float sdRoundBox( vec3 p, vec3 b, float r )
{
  vec3 q = abs(p) - b;
  return length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0) - r;
}

vec2 map(vec3 p)
{
    float objID = 0.0;
    vec2 res = vec2(sdTorus(p-vec3(0.0, 0.0, -2.0), vec2(1.0, 0.3)), objID++);
    res = opU(res, vec2(sdRoundBox(p-vec3(1.5, 0.0, 0.5), vec3(0.7, 0.7, 0.7), 0.2), objID++));
    res = opU(res, vec2(sdOctahedron(p-vec3(-1.5, 0.2, 0.5), 1.2), objID++));
    res = opU(res, vec2(p.y + 1.0, objID++));    
    return res;
}

vec3 calculateTransmittance(vec3 ro, vec3 rd, float tmin, float tmax, float atten)
{
    const int MAX_DEPTH = 4;
    float hitPoints[MAX_DEPTH];
    int depth = 0;
    
    for (float t = tmin; t < tmax;)
    {
        float h = abs(map(ro + t * rd).x);
        if (h < 1e-5) { hitPoints[depth++] = t; t += 0.01; };
        if (depth >= MAX_DEPTH) break;
        t += h;
    }
    
    float thickness = 0.0;
    for (int i = 0; i < depth - 1; i += 2) thickness += hitPoints[i+1] - hitPoints[i];
    
    return vec3(1.0) * exp(-atten * thickness * thickness);
}

vec2 rayMarch(vec3 ro, vec3 rd, float tmin, float tmax)
{
	for (float t = tmin; t < tmax;)
    {
        vec3 pos = ro + t * rd;
        vec2 res = map(pos);
        
        float dist = res.x;
        if (dist < 0.001) return vec2(t, res.y);
        
        t += dist;
    }
    return vec2(-1.0, -1.0);
}

vec3 calculateNormal(vec3 p)
{
    vec3 dt = vec3(0.001, 0.0, 0.0);
    return normalize( vec3 ( map(p+dt.xyy).x - map(p-dt.xyy).x,
    						 map(p+dt.yxy).x - map(p-dt.yxy).x,
                             map(p+dt.yyx).x - map(p-dt.yyx).x ) );
}

vec3 render(vec3 ro, vec3 rd)
{
	vec3 color = vec3(0.0);
    
    vec2 res = rayMarch(ro, rd, 0.01, 20.0);
    float t = res.x;
    float objID = res.y;
    
    if (objID > -0.5)
    {
        vec3 pos = ro + t * rd;
        vec3 nor = calculateNormal(pos);
        
        vec3 lightDir = normalize(vec3(1.0, 1.5, -1.0));
        vec3 lightColor = vec3(1.0);
        Material mat = materials[int(objID)];
        
        float t = clamp(0.5 + 0.5 * sin(iTime), 0.2, 1.0);
        vec3 light = t * lightColor * calculateTransmittance(pos+nor*vec3(0.01), lightDir, 0.01, 10.0, 2.0);
        light += (1.0 - t) * calculateTransmittance(pos+nor*vec3(0.01), rd, 0.01, 10.0, 0.5);
        color =  light * mat.diffuseAlbedo;
        color += light * mat.specularAlbedo * pow(max(0.0, dot(reflect(lightDir,nor),rd)), 4.0);
    }
    
    return color;
}

mat3 lookAt(vec3 eye, vec3 target, vec3 up)
{
    vec3 w = normalize(target - eye);
    vec3 u = normalize(cross(w, up));
    vec3 v = cross(u, w);
    return mat3(u, v, -w);
}

void mainImage(out vec4 fragColor, in vec2 fragCoord)
{
    float an = iMouse.z>0.0 ? 10.0*(iMouse.x/iResolution.x-0.5) : 0.0;
    
	vec3 ro = 5.0*vec3(sin(an), 0.3, cos(an));
    mat3 viewMat = lookAt(ro, vec3(0.0, -1.0, 0.0), vec3(0.0, 1.0, 0.0));

    vec3 color = vec3(0.0);
    for (int i = 0; i < AA; ++i)
    {
        for (int k = 0; k < AA; ++k)
        {
            vec2 offset = vec2(float(i)+0.5,float(k)+0.5) / float(AA) - 0.5;
            vec2 uv = (2.0*(fragCoord+offset)-iResolution.xy)/iResolution.y;    
    	    vec3 rd = viewMat * normalize(vec3(uv, -1.5));
    		
            color += render(ro, rd);
        }
    }
    
	color /= float(AA*AA);
	color = pow(color, vec3(0.4545));
    
    fragColor = vec4(color, 1.0);
}