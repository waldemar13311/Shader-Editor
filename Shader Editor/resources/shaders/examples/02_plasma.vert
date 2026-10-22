#version 330 core

in vec3 position;
in vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 vPosition;

void main(void)
{
    vPosition = position;

    gl_Position = projection * view * model * vec4(position, 1.0);
}
