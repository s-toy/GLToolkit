vec3 computeReflectColor()
{
	vec3 ViewDirW = normalize(uViewPos - _inPositionW.xyz);
	vec3 NormalW = normalize(_inNormalW);

	SMaterial Material;
	Material.Diffuse = uDiffuseColor; //texture(uMaterialDiffuse, _inTexCoord).rgb;
	Material.Specular = vec3(0.0); // texture(uMaterialSpecular, _inTexCoord).rgb;
	Material.Shinness = 32.0;

	vec3 color = vec3(0.0);
	color += computePhongShading4ParallelLight(_inPositionW.xyz, NormalW, ViewDirW, SParallelLight(vec3(0.7), vec3(1.0, 1.0, 1.0)), Material);
	color += computePhongShading4ParallelLight(_inPositionW.xyz, NormalW, ViewDirW, SParallelLight(vec3(0.7), vec3(-1.0, 1.0, -1.0)), Material);

	return color;
}