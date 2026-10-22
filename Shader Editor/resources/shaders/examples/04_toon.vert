#version 330 core

in vec3 position;
in vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMatrix;

out vec3 vNormalView;
out vec3 vViewPos;

void main(void)
{
    // Нормаль переводим сразу в ПРОСТРАНСТВО КАМЕРЫ: в нём камера всегда
    // находится в начале координат, поэтому свет, направленный в +Z,
    // всегда освещает видимую сторону модели с любого ракурса.
    // (mat3(view) допустим, так как view-матрица не содержит масштаба.)
    vNormalView = normalize(mat3(view) * (normalMatrix * normal));

    vec4 viewPos = view * model * vec4(position, 1.0);
    vViewPos = viewPos.xyz;

    gl_Position = projection * viewPos;
}
