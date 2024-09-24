#version 440

layout(location = 0) in vec3 aPos;

uniform mat4 proj;

void main() 
{
    gl_Position = proj * vec4(aPos, 1);
}
