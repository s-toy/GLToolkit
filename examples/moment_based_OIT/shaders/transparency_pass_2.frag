#version 430 core

struct SParallelLight { vec3 Color; vec3 Direction; };

struct SMaterial { vec3 Diffuse; vec3 Specular; float Shinness; };

#define BLENDING_STRATEGY_PERFECT_SORTING		0
#define BLENDING_STRATEGY_AVERAGE_BLENDING		1
#define BLENDING_STRATEGY_PHNOMENOLOGICAL_OIT	2
#define BLENDING_STRATEGY_MBOIT_POWER4			3
#define BLENDING_STRATEGY_MBOIT_POWER6			4
#define BLENDING_STRATEGY_MBOIT_POWER8			5

uniform int uBlendingStrategy = BLENDING_STRATEGY_AVERAGE_BLENDING;

uniform sampler2D	uMaterialDiffuse;
uniform sampler2D	uMaterialSpecular;
uniform sampler2D   uOpaqueDepthTex;

uniform sampler2D	uMomentB0Tex;
uniform sampler2D	uMomentsTex;
uniform sampler2D	uExtraMomentsTex;

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

void sincos(float x, out float s, out float c) { s = sin(x); c = cos(x); }

vec3 SolveCubic(vec4 Coefficient) {
	// Normalize the polynomial
	Coefficient.xyz /= Coefficient.w;
	// Divide middle coefficients by three
	Coefficient.yz /= 3.0f;
	// Compute the Hessian and the discrimant
	vec3 Delta = vec3(
		mad(-Coefficient.z, Coefficient.z, Coefficient.y),
		mad(-Coefficient.y, Coefficient.z, Coefficient.x),
		dot(vec2(Coefficient.z, -Coefficient.y), Coefficient.xy)
		);
	float Discriminant = dot(vec2(4.0f*Delta.x, -Delta.y), Delta.zy);
	// Compute coefficients of the depressed cubic 
	// (third is zero, fourth is one)
	vec2 Depressed = vec2(
		mad(-2.0f*Coefficient.z, Delta.x, Delta.y),
		Delta.x
		);
	// Take the cubic root of a normalized complex number
	float Theta = atan(sqrt(Discriminant), -Depressed.x) / 3.0f;
	vec2 CubicRoot;
	sincos(Theta, CubicRoot.y, CubicRoot.x);
	// Compute the three roots, scale appropriately and 
	// revert the depression transform
	vec3 Root = vec3(
		CubicRoot.x,
		dot(vec2(-0.5f, -0.5f*sqrt(3.0f)), CubicRoot),
		dot(vec2(-0.5f, 0.5f*sqrt(3.0f)), CubicRoot)
		);
	Root = 2.0f*sqrt(-Depressed.y) * Root - Coefficient.z;
	return Root;
}

/*! Given coefficients of a quadratic polynomial A*x^2+B*x+C, this function	
	outputs its two real roots.*/
vec2 solveQuadratic(vec3 coeffs)
{
	coeffs[1] *= 0.5;

	float x1, x2, tmp;

	tmp = (coeffs[1] * coeffs[1] - coeffs[0] * coeffs[2]);
	if (coeffs[1] >= 0) {
		tmp = sqrt(tmp);
		x1 = (-coeffs[2]) / (coeffs[1] + tmp);
		x2 = (-coeffs[1] - tmp) / coeffs[0];
	} else {
		tmp = sqrt(tmp);
		x1 = (-coeffs[1] + tmp) / coeffs[0];
		x2 = coeffs[2] / (-coeffs[1] + tmp);
	}
	return vec2(x1, x2);
}


/*! Given coefficients of a cubic polynomial 
	coeffs[0]+coeffs[1]*x+coeffs[2]*x^2+coeffs[3]*x^3 with three real roots, 
	this function returns the root of least magnitude.*/
float solveCubicBlinnSmallest(vec4 coeffs)
{
	coeffs.xyz /= coeffs.w;
	coeffs.yz /= 3.0;

	vec3 delta = vec3(mad(-coeffs.z, coeffs.z, coeffs.y), mad(-coeffs.z, coeffs.y, coeffs.x), coeffs.z * coeffs.x - coeffs.y * coeffs.y);
	float discriminant = 4.0 * delta.x * delta.z - delta.y * delta.y;

	vec2 depressed = vec2(delta.z, -coeffs.x * delta.y + 2.0 * coeffs.y * delta.z);
	float theta = abs(atan(coeffs.x * sqrt(discriminant), -depressed.y)) / 3.0;
	vec2 sin_cos;
	sincos(theta, sin_cos.x, sin_cos.y);
	float tmp = 2.0 * sqrt(-depressed.x);
	vec2 x = vec2(tmp * sin_cos.y, tmp * (-0.5 * sin_cos.y - 0.5 * sqrt(3.0) * sin_cos.x));
	vec2 s = (x.x + x.y < 2.0 * coeffs.y) ? vec2(-coeffs.x, x.x + coeffs.y) : vec2(-coeffs.x, x.y + coeffs.y);

	return  s.x / s.y;
}

/*! Given coefficients of a quartic polynomial 
	coeffs[0]+coeffs[1]*x+coeffs[2]*x^2+coeffs[3]*x^3+coeffs[4]*x^4 with four 
	real roots, this function returns all roots.*/
vec4 solveQuarticNeumark(float coeffs[5])
{
	// Normalization
	float B = coeffs[3] / coeffs[4];
	float C = coeffs[2] / coeffs[4];
	float D = coeffs[1] / coeffs[4];
	float E = coeffs[0] / coeffs[4];

	// Compute coefficients of the cubic resolvent
	float P = -2.0*C;
	float Q = C*C + B*D - 4.0*E;
	float R = D*D + B*B*E -B*C*D;

	// Obtain the smallest cubic root
	float y = solveCubicBlinnSmallest(vec4(R, Q, P, 1.0));

	float BB = B*B;
	float fy = 4.0 * y;
	float BB_fy = BB - fy;

	float Z = C - y;
	float ZZ = Z*Z;
	float fE = 4.0 * E;
	float ZZ_fE = ZZ - fE;

	float G, g, H, h;
	// Compute the coefficients of the quadratics adaptively using the two 
	// proposed factorizations by Neumark. Choose the appropriate 
	// factorizations using the heuristic proposed by Herbison-Evans.
	if(y < 0 || (ZZ + fE) * BB_fy > ZZ_fE * (BB + fy)) {
		float tmp = sqrt(BB_fy);
		G = (B + tmp) * 0.5;
		g = (B - tmp) * 0.5;

		tmp = (B*Z - 2.0*D) / (2.0*tmp);
		H = mad(Z, 0.5, tmp);
		h = mad(Z, 0.5, -tmp);
	} else {
		float tmp = sqrt(ZZ_fE);
		H = (Z + tmp) * 0.5;
		h = (Z - tmp) * 0.5;

		tmp = (B*Z - 2.0*D) / (2.0*tmp);
		G = mad(B, 0.5, tmp);
		g = mad(B, 0.5, -tmp);
	}
	// Solve the quadratics
	return vec4(solveQuadratic(vec3(1.0, G, H)), solveQuadratic(vec3(1.0, g, h)));
}

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

float computeTransmittanceAtDepthFrom6PowerMoments(float b_0, vec3 b_even, vec3 b_odd, float depth, float bias, float overestimation, float bias_vector[6])
{
	float b[6] = { b_odd.x, b_even.x, b_odd.y, b_even.y, b_odd.z, b_even.z };
	// Bias input data to avoid artifacts
	 for (int i = 0; i != 6; ++i) {
		b[i] = mix(b[i], bias_vector[i], bias);
	}

	vec4 z;
	z[0] = depth;

	// Compute a Cholesky factorization of the Hankel matrix B storing only non-
	// trivial entries or related products
	float InvD11 = 1.0f / mad(-b[0], b[0], b[1]);
	float L21D11 = mad(-b[0], b[1], b[2]);
	float L21 = L21D11*InvD11;
	float D22 = mad(-L21D11, L21, mad(-b[1], b[1], b[3]));
	float L31D11 = mad(-b[0], b[2], b[3]);
	float L31 = L31D11*InvD11;
	float InvD22 = 1.0f / D22;
	float L32D22 = mad(-L21D11, L31, mad(-b[1], b[2], b[4]));
	float L32 = L32D22*InvD22;
	float D33 = mad(-b[2], b[2], b[5]) - dot(vec2(L31D11, L32D22), vec2(L31, L32));
	float InvD33 = 1.0f / D33;

	// Construct the polynomial whose roots have to be points of support of the 
	// canonical distribution: bz=(1,z[0],z[0]*z[0],z[0]*z[0]*z[0])^T
	vec4 c;
	c[0] = 1.0f;
	c[1] = z[0];
	c[2] = c[1] * z[0];
	c[3] = c[2] * z[0];
	// Forward substitution to solve L*c1=bz
	c[1] -= b[0];
	c[2] -= mad(L21, c[1], b[1]);
	c[3] -= b[2] + dot(vec2(L31, L32), c.yz);
	// Scaling to solve D*c2=c1
	c.yzw *= vec3(InvD11, InvD22, InvD33);
	// Backward substitution to solve L^T*c3=c2
	c[2] -= L32*c[3];
	c[1] -= dot(vec2(L21, L31), c.zw);
	c[0] -= dot(vec3(b[0], b[1], b[2]), c.yzw);

	// Solve the cubic equation
	z.yzw = SolveCubic(c);

	// Compute the absorbance by summing the appropriate weights
	vec4 weigth_factor;
	weigth_factor[0] = overestimation;
	weigth_factor.y = z.y > z.x ? 0.0f : 1.0f;
	weigth_factor.z = z.z > z.x ? 0.0f : 1.0f;
	weigth_factor.w = z.w > z.x ? 0.0f : 1.0f;
	// Construct an interpolation polynomial
	float f0 = weigth_factor[0];
	float f1 = weigth_factor[1];
	float f2 = weigth_factor[2];
	float f3 = weigth_factor[3];
	float f01 = (f1 - f0) / (z[1] - z[0]);
	float f12 = (f2 - f1) / (z[2] - z[1]);
	float f23 = (f3 - f2) / (z[3] - z[2]);
	float f012 = (f12 - f01) / (z[2] - z[0]);
	float f123 = (f23 - f12) / (z[3] - z[1]);
	float f0123 = (f123 - f012) / (z[3] - z[0]);
	vec4 polynomial;
	// f012+f0123 *(z-z2)
	polynomial[0] = mad(-f0123, z[2], f012);
	polynomial[1] = f0123;
	// *(z-z1) +f01
	polynomial[2] = polynomial[1];
	polynomial[1] = mad(polynomial[1], -z[1], polynomial[0]);
	polynomial[0] = mad(polynomial[0], -z[1], f01);
	// *(z-z0) +f0
	polynomial[3] = polynomial[2];
	polynomial[2] = mad(polynomial[2], -z[0], polynomial[1]);
	polynomial[1] = mad(polynomial[1], -z[0], polynomial[0]);
	polynomial[0] = mad(polynomial[0], -z[0], f0);
	float absorbance = dot(polynomial, vec4 (1.0, b[0], b[1], b[2]));
	// Turn the normalized absorbance into transmittance
	return clamp(exp(-b_0 * absorbance), 0.0, 1.0);
}

/*! This function reconstructs the transmittance at the given depth from eight 
	normalized power moments and the given zeroth moment.*/
float computeTransmittanceAtDepthFrom8PowerMoments(float b_0, vec4 b_even, vec4 b_odd, float depth, float bias, float overestimation, float bias_vector[8])
{
	float b[8] = { b_odd.x, b_even.x, b_odd.y, b_even.y, b_odd.z, b_even.z, b_odd.w, b_even.w };
	// Bias input data to avoid artifacts
	for (int i = 0; i != 8; ++i) {
		b[i] = mix(b[i], bias_vector[i], bias);
	}

	float z[5];
	z[0] = depth;

	// Compute a Cholesky factorization of the Hankel matrix B storing only non-trivial entries or related products
	float D22 = mad(-b[0], b[0], b[1]);
	float InvD22 = 1.0 / D22;
	float L32D22 = mad(-b[1], b[0], b[2]);
	float L32 = L32D22 * InvD22;
	float L42D22 = mad(-b[2], b[0], b[3]);
	float L42 = L42D22 * InvD22;
	float L52D22 = mad(-b[3], b[0], b[4]);
	float L52 = L52D22 * InvD22;

	float D33 = mad(-L32, L32D22, mad(-b[1], b[1], b[3]));
	float InvD33 = 1.0 / D33;
	float L43D33 = mad(-L42, L32D22, mad(-b[2], b[1], b[4]));
	float L43 = L43D33 * InvD33;
	float L53D33 = mad(-L52, L32D22, mad(-b[3], b[1], b[5]));
	float L53 = L53D33 * InvD33;

	float D44 = mad(-b[2], b[2], b[5]) - dot(vec2(L42, L43), vec2(L42D22, L43D33));
	float InvD44 = 1.0 / D44;
	float L54D44 = mad(-b[3], b[2], b[6]) - dot(vec2(L52, L53), vec2(L42D22, L43D33));
	float L54 = L54D44 * InvD44;

	float D55 = mad(-b[3], b[3], b[7]) - dot(vec3(L52, L53, L54), vec3(L52D22, L53D33, L54D44));
	float InvD55 = 1.0 / D55;

	// Construct the polynomial whose roots have to be points of support of the
	// Canonical distribution:
	// bz = (1,z[0],z[0]^2,z[0]^3,z[0]^4)^T
	float c[5];
	c[0] = 1.0;
	c[1] = z[0];
	c[2] = c[1] * z[0];
	c[3] = c[2] * z[0];
	c[4] = c[3] * z[0];

	// Forward substitution to solve L*c1 = bz
	c[1] -= b[0];
	c[2] -= mad(L32, c[1], b[1]);
	c[3] -= b[2] + dot(vec2(L42, L43), vec2(c[1], c[2]));
	c[4] -= b[3] + dot(vec3(L52, L53, L54), vec3(c[1], c[2], c[3]));

	// Scaling to solve D*c2 = c1
	//c = c .*[1, InvD22, InvD33, InvD44, InvD55];
	c[1] *= InvD22;
	c[2] *= InvD33;
	c[3] *= InvD44;
	c[4] *= InvD55;

	// Backward substitution to solve L^T*c3 = c2
	c[3] -= L54 * c[4];
	c[2] -= dot(vec2(L53, L43), vec2(c[4], c[3]));
	c[1] -= dot(vec3(L52, L42, L32), vec3(c[4], c[3], c[2]));
	c[0] -= dot(vec4(b[3], b[2], b[1], b[0]), vec4(c[4], c[3], c[2], c[1]));

	// Solve the quartic equation
	vec4 zz = solveQuarticNeumark(c);
	z[1] = zz[0];
	z[2] = zz[1];
	z[3] = zz[2];
	z[4] = zz[3];

	// Compute the absorbance by summing the appropriate weights
	vec4 weigth_factor;
	weigth_factor.x = z[1] <= z[0] ? 1.0 : 0.0;
	weigth_factor.y = z[2] <= z[0] ? 1.0 : 0.0;
	weigth_factor.z = z[3] <= z[0] ? 1.0 : 0.0;
	weigth_factor.w = z[4] <= z[0] ? 1.0 : 0.0;
	// Construct an interpolation polynomial
	float f0 = overestimation;
	float f1 = weigth_factor[0];
	float f2 = weigth_factor[1];
	float f3 = weigth_factor[2];
	float f4 = weigth_factor[3];
	float f01 = (f1 - f0) / (z[1] - z[0]);
	float f12 = (f2 - f1) / (z[2] - z[1]);
	float f23 = (f3 - f2) / (z[3] - z[2]);
	float f34 = (f4 - f3) / (z[4] - z[3]);
	float f012 = (f12 - f01) / (z[2] - z[0]);
	float f123 = (f23 - f12) / (z[3] - z[1]);
	float f234 = (f34 - f23) / (z[4] - z[2]);
	float f0123 = (f123 - f012) / (z[3] - z[0]);
	float f1234 = (f234 - f123) / (z[4] - z[1]);
	float f01234 = (f1234 - f0123) / (z[4] - z[0]);

	float Polynomial_0;
	vec4 Polynomial;
	// f0123 + f01234 * (z - z3)
	Polynomial_0 = mad(-f01234, z[3], f0123);
	Polynomial[0] = f01234;
	// * (z - z2) + f012
	Polynomial[1] = Polynomial[0];
	Polynomial[0] = mad(-Polynomial[0], z[2], Polynomial_0);
	Polynomial_0 = mad(-Polynomial_0, z[2], f012);
	// * (z - z1) + f01
	Polynomial[2] = Polynomial[1];
	Polynomial[1] = mad(-Polynomial[1], z[1], Polynomial[0]);
	Polynomial[0] = mad(-Polynomial[0], z[1], Polynomial_0);
	Polynomial_0 = mad(-Polynomial_0, z[1], f01);
	// * (z - z0) + f1
	Polynomial[3] = Polynomial[2];
	Polynomial[2] = mad(-Polynomial[2], z[0], Polynomial[1]);
	Polynomial[1] = mad(-Polynomial[1], z[0], Polynomial[0]);
	Polynomial[0] = mad(-Polynomial[0], z[0], Polynomial_0);
	Polynomial_0 = mad(-Polynomial_0, z[0], f0);
	float absorbance = Polynomial_0 + dot(Polynomial, vec4(b[0], b[1], b[2], b[3]));
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
	else if (uBlendingStrategy == BLENDING_STRATEGY_MBOIT_POWER4)
	{
		float b_0 = texelFetch(uMomentB0Tex, ivec2(gl_FragCoord.xy), 0).x;
		if (b_0 < 0.00100050033f) discard;
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
	else if (uBlendingStrategy == BLENDING_STRATEGY_MBOIT_POWER6)
	{
		float b_0 = texelFetch(uMomentB0Tex, ivec2(gl_FragCoord.xy), 0).x;
		if (b_0 < 0.00100050033f) discard;
		vec4  b_1234 = texelFetch(uMomentsTex, ivec2(gl_FragCoord.xy), 0).xyzw;
		vec2  b_56 = texelFetch(uExtraMomentsTex, ivec2(gl_FragCoord.xy), 0).xy;
		vec3  b_even = vec3(b_1234.yw, b_56.y);
		vec3  b_odd  = vec3(b_1234.xz, b_56.x);
		b_even /= b_0;
		b_odd /= b_0;

		const float bias_vector[6] = { 0, 0.48, 0, 0.451, 0, 0.45 };
		float depth = 2.0 * gl_FragCoord.z - 1.0;
		float moment_bias = 5e-6;
		float overestimation = 0.25;

		transmittance_at_depth = computeTransmittanceAtDepthFrom6PowerMoments(b_0, b_even, b_odd, depth, moment_bias, overestimation, bias_vector);
	}
	else if (uBlendingStrategy == BLENDING_STRATEGY_MBOIT_POWER8)
	{
		float b_0 = texelFetch(uMomentB0Tex, ivec2(gl_FragCoord.xy), 0).x;
		if (b_0 < 0.00100050033f) discard;
		vec4  b_1234 = texelFetch(uMomentsTex, ivec2(gl_FragCoord.xy), 0).xyzw;
		vec4  b_5678 = texelFetch(uExtraMomentsTex, ivec2(gl_FragCoord.xy), 0).xyzw;
		vec4  b_even = vec4(b_1234.yw, b_5678.yw);
		vec4  b_odd  = vec4(b_1234.xz, b_5678.xz);
		b_even /= b_0;
		b_odd /= b_0;

		const float bias_vector[8] = { 0, 0.75, 0, 0.67666666666666664, 0, 0.63, 0, 0.60030303030303034 };
		float depth = 2.0 * gl_FragCoord.z - 1.0;
		float moment_bias = 5e-5;
		float overestimation = 0.25;

		transmittance_at_depth = computeTransmittanceAtDepthFrom8PowerMoments(b_0, b_even, b_odd, depth, moment_bias, overestimation, bias_vector);
	}

	vec3 reflectColor = computeReflectColor();
	_outTransparencyColor.rgb = transmittance_at_depth * uCoverage * reflectColor;
	_outTransparencyColor.a = transmittance_at_depth * uCoverage;
}