#version 330 core

in vec3 position;
in vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMatrix;
uniform float time;

out float vHeight;
out vec3 vNormal;

// Пример 3. Волны.
// Вершинный шейдер сдвигает каждую вершину по вертикали (ось Z в этой сцене)
// синусоидой, зависящей от времени и позиции. Геометрия должна быть достаточно
// плотной - на грубых моделях волна будет угловатой.

void main(void)
{
    vec3 displaced = position;
    displaced.z += 0.1 * sin(time * 3.0 + position.x * 6.0 + position.y * 4.0);

    // важна именно величина волны (смещения), а не абсолютная высота вершины:
    // иначе градиент цвета сожмётся в узкую полосу около нуля
    vHeight = displaced.z - position.z;
    vNormal = normalize(normalMatrix * normal);

    gl_Position = projection * view * model * vec4(displaced, 1.0);
}
