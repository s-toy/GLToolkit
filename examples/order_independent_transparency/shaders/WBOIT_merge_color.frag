#version 430 core

uniform sampler2D uOpaqueColorTex;
uniform sampler2D uAccumulatedTranslucentColorTex;
uniform sampler2D uAccumulatedTransmittanceTex;

void main()
{
	ivec2 uv = ivec2(gl_FragCoord.xy);
	vec3  opaqueColor = texelFetch(uOpaqueColorTex, uv, 0).rgb;
	vec4  translucentColor = texelFetch(uAccumulatedTranslucentColorTex, uv, 0).rgba;
	vec3  totalTransmittance = exp(-texelFetch(uAccumulatedTransmittanceTex, uv, 0).rgb);
	vec3  finalColor = mix(translucentColor.rgb / (translucentColor.a + 1e-5), opaqueColor, totalTransmittance);

	gl_FragColor = vec4(finalColor, 1.0);
}