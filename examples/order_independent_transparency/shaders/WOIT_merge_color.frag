#version 460 core
#include "global_macro.h"

uniform sampler2D uOpaqueColorTex;
uniform sampler2D uTranslucentColorTex;
uniform sampler2D uTotalAbsorbanceTex;

void main()
{
	ivec2 fragCoord = ivec2(gl_FragCoord.xy);
	vec2  texCoord = fragCoord / vec2(WIN_WIDTH, WIN_HEIGHT);

	vec3  opaqueColor = texelFetch(uOpaqueColorTex, fragCoord, 0).rgb;

	vec4  translucentColor = texture(uTranslucentColorTex, texCoord).rgba;

	float totalOpticalDepth = texture(uTotalAbsorbanceTex, texCoord).r;
	float totalTransmittance = exp(-totalOpticalDepth);

	vec3 finalColor = mix(translucentColor.rgb / (translucentColor.a + 1e-5), opaqueColor, totalTransmittance);
	//vec3 finalColor = mix(translucentColor.rgb, opaqueColor, totalTransmittance);
	//vec3 finalColor = translucentColor.rgb + opaqueColor * 0.4f;
	gl_FragColor = vec4(finalColor, 1.0);
}