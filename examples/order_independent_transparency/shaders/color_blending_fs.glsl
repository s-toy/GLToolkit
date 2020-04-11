#version 430 core

uniform sampler2D uOpaqueColorTex;

layout(location = 0) in vec2 _inTexCoord;

void main()
{
	vec3 color = texture(uOpaqueColorTex, _inTexCoord).rgb;

	gl_FragColor = vec4(color, 1.0);
}