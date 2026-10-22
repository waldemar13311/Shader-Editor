#version 330 core

in vec3 position;
in vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMatrix;

out vec3 vNormal;

void main(void)
{
    vNormal = normalize(normalMatrix * normal);

    gl_Position = projection * view * model * vec4(position, 1.0);
}
