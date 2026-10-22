#version 330 core

in vec3 vPosition;

uniform float time;

out vec4 FragColor;

// Пример 2. Плазма.
// Анимированная радужная текстура из суммы синусоид. Параметры можно крутить:
// множители при time - скорость анимации, множитель при p - масштаб узора.

void main()
{
    vec2 p = vPosition.xy * 3.0;

    float v = sin(p.x + time * 1.5)
            + sin((p.y + time) * 1.2)
            + sin((p.x + p.y + time * 2.0) * 0.8)
            + sin(length(p + vec2(time, time * 0.5)) * 2.0);

    FragColor = vec4(0.5 + 0.5 * cos(vec3(0.0, 2.1, 4.2) + v), 1.0);
}
