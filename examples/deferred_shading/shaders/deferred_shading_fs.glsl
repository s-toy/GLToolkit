#version 430 core

uniform sampler2D uPositionTex;
uniform sampler2D uNormalTex;
uniform sampler2D uDiffuseTex;
uniform sampler2D uSpecularTex;

uniform vec3 uViewPos;

layout(location = 0) in vec2 _inTexCoord;

void main()
{
	vec3 position = texture(uPositionTex, _inTexCoord).xyz;
	vec3 normal = texture(uNormalTex, _inTexCoord).xyz;
	vec3 diffuse = texture(uDiffuseTex, _inTexCoord).rgb;
	vec3 specular = texture(uSpecularTex, _inTexCoord).rgb;
	vec3 lightDir = normalize(vec3(1, 1, 1));
	vec3 viewDir = normalize(uViewPos - position);

	vec3 color = 0.05 * diffuse;
	color += 0.2 * diffuse * max(dot(normal, lightDir), 0);
	color += 0.5 * specular * pow(max(dot(viewDir, normalize(reflect(-lightDir, normal))), 0), 32.0);

	gl_FragColor = vec4(color, 1.0);
}