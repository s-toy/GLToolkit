#version 460 core

uniform sampler2D uOpaqueColorTex;
uniform sampler2D uTranslucentColorTex;
uniform sampler2D uWaveletOpacityMap1;

void main()
{
	ivec2 uv = ivec2(gl_FragCoord.xy);
	vec3  opaqueColor = texelFetch(uOpaqueColorTex, uv, 0).rgb;
	vec4  translucentColor = texelFetch(uTranslucentColorTex, uv, 0).rgba;

	float totalOpticalDepth = texelFetch(uWaveletOpacityMap1, uv, 0).x;
	float totalTransmittance = exp(-totalOpticalDepth);

	vec3 finalColor = mix(translucentColor.rgb / (translucentColor.a + 1e-5), opaqueColor, totalTransmittance);
	gl_FragColor = vec4(finalColor, 1.0);
}