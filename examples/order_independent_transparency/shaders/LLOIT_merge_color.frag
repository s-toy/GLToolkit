#version 430 core

uniform sampler2D uOpaqueColorTex;
uniform sampler2D uTransparentColorTex;

void main()
{
	ivec2 uv = ivec2(gl_FragCoord.xy);
	vec3 opaqueColor = texelFetch(uOpaqueColorTex, uv, 0).rgb;
	vec4 transparentColor = texelFetch(uTransparentColorTex, uv, 0).rgba;
	
	vec3 color = transparentColor.rgb + opaqueColor * transparentColor.a;

	gl_FragColor = vec4(color, 1.0);
}