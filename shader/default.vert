#version 440

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

layout (std140, binding = 1) uniform matrices
{
    mat4 projection;
    mat4 view;
};

out vec3 color;

void main() 
{
    color = aColor;
    gl_Position = projection * view * vec4(aPos, 1);
}
