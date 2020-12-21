uniform vec3	uMaterialDiffuse;
uniform vec3	uMaterialSpecular;

vec3 ACESFilmToneMapping(vec3 x)
{
	float a = 2.51f;
	float b = 0.03f;
	float c = 2.43f;
	float d = 0.59f;
	float e = 0.14f;
	return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

vec3 computeReflectColor()
{
	vec3 ViewDirW = normalize(uViewPos - _inPositionW.xyz);
	vec3 NormalW = normalize(_inNormalW);

	SMaterial Material;
	Material.Diffuse = uMaterialDiffuse; //texture(uMaterialDiffuseTex, _inTexCoord).rgb;
	Material.Specular = vec3(0.6); //texture(uMaterialSpecularTex, _inTexCoord).rgb;
	Material.Shinness = 32.0;

	vec3 color = vec3(0.0);
	color += computePhongShading4ParallelLight(_inPositionW.xyz, NormalW, ViewDirW, SParallelLight(vec3(0.7), vec3(1.0, 1.0, 1.0)), Material);
	color += computePhongShading4ParallelLight(_inPositionW.xyz, NormalW, ViewDirW, SParallelLight(vec3(0.7), vec3(-1.0, 1.0, -1.0)), Material);
	color = ACESFilmToneMapping(color);

	return color;
}