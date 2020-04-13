#version 430 core

uniform sampler2D uOpaqueColorTex;
uniform sampler2D uTransparentColorTex;

layout(location = 0) in vec2 _inTexCoord;

void main()
{
	vec3 opaqueColor = texture(uOpaqueColorTex, _inTexCoord).rgb;
	vec4 transparentColor = texture(uTransparentColorTex, _inTexCoord).rgba;
	
	vec3 color = transparentColor.rgb + opaqueColor * transparentColor.a;

	gl_FragColor = vec4(color, 1.0);
}