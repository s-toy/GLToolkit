#version 430 core

uniform sampler2D uOpaqueColorTex;
uniform sampler2D uAccumulatedReflectionTex;
uniform sampler2D uAccumulatedTransmissionTex;

layout(location = 0) in vec2 _inTexCoord;

void main()
{
	vec3 bgColor = texture(uOpaqueColorTex, _inTexCoord).rgb;
	vec4 A = texture(uAccumulatedReflectionTex, _inTexCoord).rgba;
	vec3 B = texture(uAccumulatedTransmissionTex, _inTexCoord).rgb;
	
	vec3 color = bgColor * B + (1.0 - B) * A.rgb / (A.a + 1e-5);
	gl_FragColor = vec4(color, 1.0);
}