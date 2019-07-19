#version 330 core

out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform vec2 screenSize;

float FXAA_SPAN_MAX = 8.0f;
float FXAA_REDUCE_MUL = 1.0f/8.0f;
float FXAA_REDUCE_MIN = 1.0f/128.0f;

vec3 computeToneMapping(vec2 texCoords);
vec3 computeFxaa();

void main()
{
    const float gamma = 2.2;

	vec3 hdrColor = computeFxaa();

	// gamma correction
	hdrColor = pow(hdrColor, vec3(1.0/gamma));
	FragColor = vec4(hdrColor, 1.0);
}

vec3 computeToneMapping(vec2 texCoords)
{
	const float exposure = 1.0;
	vec3 hdrColor = texture(screenTexture, texCoords).xyz;
	return vec3(1.0) - exp(-hdrColor * exposure);
}

vec3 computeFxaa()
{
    vec2 screenTextureOffset = 1.0 / screenSize;
    vec3 luma = vec3(0.299f, 0.587f, 0.114f);

    vec3 offsetNW = computeToneMapping(TexCoords.xy + (vec2(-1.0f, -1.0f) * screenTextureOffset));
    vec3 offsetNE = computeToneMapping(TexCoords.xy + (vec2(1.0f, -1.0f) * screenTextureOffset));
    vec3 offsetSW = computeToneMapping(TexCoords.xy + (vec2(-1.0f, 1.0f) * screenTextureOffset));
    vec3 offsetSE = computeToneMapping(TexCoords.xy + (vec2(1.0f, 1.0f) * screenTextureOffset));
    vec3 offsetM  = computeToneMapping(TexCoords.xy);

    float lumaNW = dot(luma, offsetNW);
    float lumaNE = dot(luma, offsetNE);
    float lumaSW = dot(luma, offsetSW);
    float lumaSE = dot(luma, offsetSE);
    float lumaM  = dot(luma, offsetNW);

    vec2 dir = vec2(-((lumaNW + lumaNE) - (lumaSW + lumaSE)), ((lumaNW + lumaSW) - (lumaNE + lumaSE)));

    float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * (FXAA_REDUCE_MUL * 0.25f), FXAA_REDUCE_MIN);
    float dirCorrection = 1.0f / (min(abs(dir.x), abs(dir.y)) + dirReduce);

    dir = min(vec2(FXAA_SPAN_MAX), max(vec2(-FXAA_SPAN_MAX), dir * dirCorrection)) * screenTextureOffset;

    vec3 resultA = 0.5f * (computeToneMapping(TexCoords.xy + (dir * vec2(1.0f / 3.0f - 0.5f))) +
                                    computeToneMapping(TexCoords.xy + (dir * vec2(2.0f / 3.0f - 0.5f))));

    vec3 resultB = resultA * 0.5f + 0.25f * (computeToneMapping(TexCoords.xy + (dir * vec2(0.0f / 3.0f - 0.5f))).xyz +
                                             computeToneMapping(TexCoords.xy + (dir * vec2(3.0f / 3.0f - 0.5f))).xyz);

    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));
    float lumaResultB = dot(luma, resultB);

    if(lumaResultB < lumaMin || lumaResultB > lumaMax) {
		return vec3(resultA);
	}

	return vec3(resultB);
}