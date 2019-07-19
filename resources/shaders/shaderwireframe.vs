#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 3) in vec2 aTexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec2 textureScale;
uniform sampler2D displacementMap;
uniform float displacementAmount;

void main()
{
	vec3 Position = aPos;
	vec2 TexCoords = aTexCoords * textureScale;

	if (displacementAmount > 0.0) {
		float k = texture(displacementMap, TexCoords).r * displacementAmount;
		Position = Position + aNormal * k;
	}

	gl_Position = projection * view * model * vec4(Position, 1.0);
};