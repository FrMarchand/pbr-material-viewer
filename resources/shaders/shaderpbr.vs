#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aTangent;
layout(location = 3) in vec2 aTexCoords;

out vec3 Normal;
out vec2 TexCoords;
out vec3 TangentViewDir;
out vec3 TangentFragPos;
out vec3 TangentLightPos;
out mat3 iTBN;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMat;
uniform vec3 eyePos;
uniform vec3 lightPos;
uniform vec2 textureScale;
uniform sampler2D displacementMap;
uniform float displacementAmount;

void main()
{
	TexCoords = aTexCoords * textureScale;
	vec3 Position = aPos;

	if (displacementAmount > 0.0) {
		float k = texture(displacementMap, TexCoords).r * displacementAmount;
		Position = Position + aNormal * k;
	}

	gl_Position = projection * view * model * vec4(Position, 1.0);

	vec3 T = normalize(vec3(normalMat * aTangent));
	vec3 N = normalize(vec3(normalMat * aNormal));
	vec3 B = cross(N, T);

	Normal = N;
	mat3 TBN = transpose(mat3(T, B, N));

	vec3 FragPos = vec3(model * vec4(Position, 1.0));
	TangentFragPos = TBN * FragPos;

	TangentViewDir = TBN * (eyePos - FragPos);


	TangentLightPos = TBN * lightPos;

	iTBN = inverse(TBN);
}