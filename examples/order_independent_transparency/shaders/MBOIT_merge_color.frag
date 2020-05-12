#version 430 core

uniform sampler2D uOpaqueColorTex;
uniform sampler2D uTranslucentColorTex;
uniform sampler2D uMomentB0Tex;

void main()
{
	ivec2 uv = ivec2(gl_FragCoord.xy);
	vec3  opaqueColor = texelFetch(uOpaqueColorTex, uv, 0).rgb;
	vec4  translucentColor = texelFetch(uTranslucentColorTex, uv, 0).rgba;
	float b0 = texelFetch(uMomentB0Tex, uv, 0).r;

	float totalTransmittance = exp(-b0);
	vec3 finalColor = mix(translucentColor.rgb / (translucentColor.a + 1e-5), opaqueColor, totalTransmittance);
	//vec3 finalColor = totalTransmittance * opaqueColor + translucentColor.rgb;

	gl_FragColor = vec4(finalColor, 1.0);
}