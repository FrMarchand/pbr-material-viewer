#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 FragPos;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    FragPos = aPos;  
    gl_Position =  projection * view * vec4(FragPos, 1.0);
}