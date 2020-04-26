#version 430 core

struct SParallelLight { vec3 Color; vec3 Direction; };

struct SMaterial { vec3 Diffuse; vec3 Specular; float Shinness; };

#define BLENDING_STRATEGY_PERFECT_SORTING		0
#define BLENDING_STRATEGY_AVERAGE_BLENDING		1
#define BLENDING_STRATEGY_PHNOMENOLOGICAL_OIT	2
#define BLENDING_STRATEGY_MOMENT_BASED_OIT		3

uniform int uBlendingStrategy = BLENDING_STRATEGY_AVERAGE_BLENDING;

uniform sampler2D	uMaterialDiffuse;
uniform sampler2D	uMaterialSpecular;
uniform sampler2D   uOpaqueDepthTex;

uniform sampler2D	uMomentB0Tex;
uniform sampler2D	uMomentsTex;

uniform vec3	uViewPos = vec3(0.0);
uniform vec3	uDiffuseColor;
uniform float	uCoverage;

layout(location = 0) in vec3 _inPositionW;
layout(location = 1) in vec3 _inNormalW;
layout(location = 2) in vec2 _inTexCoord;

layout(location = 0) out vec4 _outTransparencyColor;

vec3 computePhongShading4ParallelLight(vec3 vPositionW, vec3 vNormalW, vec3 vViewDir, SParallelLight vLight, SMaterial vMaterial)
{
	vec3 AmbientColor = 0.2 * vLight.Color * vMaterial.Diffuse;
	vec3 DiffuseColor = vLight.Color * vMaterial.Diffuse * max(dot(vNormalW, vLight.Direction), 0.0);
	vec3 ReflectDir = normalize(reflect(-vLight.Direction, _inNormalW));
	vec3 SpecularColor = vLight.Color * vMaterial.Specular * pow(max(dot(vViewDir, ReflectDir), 0.0), vMaterial.Shinness);

	return AmbientColor + DiffuseColor /*+ SpecularColor*/;
}

vec3 computeReflectColor()
{
	vec3 ViewDirW = normalize(uViewPos - _inPositionW);
	vec3 NormalW = normalize(_inNormalW);

	SMaterial Material;
	Material.Diffuse = uDiffuseColor; // texture(uMaterialDiffuse, _inTexCoord).rgb;
	Material.Specular = vec3(0.0); // texture(uMaterialSpecular, _inTexCoord).rgb;
	Material.Shinness = 32.0;

	vec3 color = vec3(0.0);
	color += computePhongShading4ParallelLight(_inPositionW, NormalW, ViewDirW, SParallelLight(vec3(0.7), vec3(1.0, 1.0, 1.0)), Material);
	color += computePhongShading4ParallelLight(_inPositionW, NormalW, ViewDirW, SParallelLight(vec3(0.7), vec3(-1.0, 1.0, -1.0)), Material);

	return color;
}

float mad(float m, float a, float b) { return m * a + b; }

/*! This function reconstructs the transmittance at the given depth from four 
	normalized power moments and the given zeroth moment.*/
float computeTransmittanceAtDepthFrom4PowerMoments(float b_0, vec2 b_even, vec2 b_odd, float depth, float bias, float overestimation, vec4 bias_vector)
{
	vec4 b = vec4(b_odd.x, b_even.x, b_odd.y, b_even.y);
	// Bias input data to avoid artifacts
	b = mix(b, bias_vector, bias);
	vec3 z;
	z[0] = depth;

	// Compute a Cholesky factorization of the Hankel matrix B storing only non-
	// trivial entries or related products
	float L21D11=mad(-b[0],b[1],b[2]);
	float D11=mad(-b[0],b[0], b[1]);
	float InvD11=1.0f/D11;
	float L21=L21D11*InvD11;
	float SquaredDepthVariance=mad(-b[1],b[1], b[3]);
	float D22=mad(-L21D11,L21,SquaredDepthVariance);

	// Obtain a scaled inverse image of bz=(1,z[0],z[0]*z[0])^T
	vec3 c=vec3(1.0f,z[0],z[0]*z[0]);
	// Forward substitution to solve L*c1=bz
	c[1]-=b.x;
	c[2]-=b.y+L21*c[1];
	// Scaling to solve D*c2=c1
	c[1]*=InvD11;
	c[2]/=D22;
	// Backward substitution to solve L^T*c3=c2
	c[1]-=L21*c[2];
	c[0]-=dot(c.yz,b.xy);
	// Solve the quadratic equation c[0]+c[1]*z+c[2]*z^2 to obtain solutions 
	// z[1] and z[2]
	float InvC2=1.0f/c[2];
	float p=c[1]*InvC2;
	float q=c[0]*InvC2;
	float D=(p*p*0.25f)-q;
	float r=sqrt(D);
	z[1]=-p*0.5f-r;
	z[2]=-p*0.5f+r;
	// Compute the absorbance by summing the appropriate weights
	vec3 polynomial;
	vec3 weight_factor = vec3(overestimation, (z[1] < z[0])?1.0f:0.0f, (z[2] < z[0])?1.0f:0.0f);
	float f0=weight_factor[0];
	float f1=weight_factor[1];
	float f2=weight_factor[2];
	float f01=(f1-f0)/(z[1]-z[0]);
	float f12=(f2-f1)/(z[2]-z[1]);
	float f012=(f12-f01)/(z[2]-z[0]);
	polynomial[0]=f012;
	polynomial[1]=polynomial[0];
	polynomial[0]=f01-polynomial[0]*z[1];
	polynomial[2]=polynomial[1];
	polynomial[1]=polynomial[0]-polynomial[1]*z[0];
	polynomial[0]=f0-polynomial[0]*z[0];
	float absorbance = polynomial[0] + dot(b.xy, polynomial.yz);;
	// Turn the normalized absorbance into transmittance
	return clamp(exp(-b_0 * absorbance), 0.0, 1.0);
}

void main()
{
	float depth = texelFetch(uOpaqueDepthTex, ivec2(gl_FragCoord.xy), 0).r;
	if (depth != 0.0 && gl_FragCoord.z > depth) discard;

	float transmittance_at_depth = 0.0;

	if (uBlendingStrategy == BLENDING_STRATEGY_PERFECT_SORTING)
	{
		//TODO
	}
	else if (uBlendingStrategy == BLENDING_STRATEGY_AVERAGE_BLENDING)
	{
		transmittance_at_depth = 1.0;
	}
	else if (uBlendingStrategy == BLENDING_STRATEGY_PHNOMENOLOGICAL_OIT)
	{
		transmittance_at_depth = pow(10.0 * (1.0 - 0.99 * gl_FragCoord.z) * uCoverage, 3.0);
		transmittance_at_depth = clamp(transmittance_at_depth, 0.01, 30.0);
	}
	else if (uBlendingStrategy == BLENDING_STRATEGY_MOMENT_BASED_OIT)
	{
		float b_0 = texelFetch(uMomentB0Tex, ivec2(gl_FragCoord.xy), 0).x;
		vec4  b_1234 = texelFetch(uMomentsTex, ivec2(gl_FragCoord.xy), 0).xyzw;
		vec2  b_even = b_1234.yw;
		vec2  b_odd = b_1234.xz;
		b_even /= b_0;
		b_odd /= b_0;

		const vec4 bias_vector = vec4(0, 0.375, 0, 0.375);
		float depth = 2.0 * gl_FragCoord.z - 1.0;
		float moment_bias = 5e-7; //recommended bias from http://momentsingraphics.de/Media/I3D2018/Muenstermann2018-MBOITSupplementary.pdf
		float overestimation = 0.25;

		transmittance_at_depth = computeTransmittanceAtDepthFrom4PowerMoments(b_0, b_even, b_odd, depth, moment_bias, overestimation, bias_vector);
	}

	vec3 reflectColor = computeReflectColor();
	_outTransparencyColor.rgb = transmittance_at_depth * uCoverage * reflectColor;
	_outTransparencyColor.a = transmittance_at_depth * uCoverage;
}