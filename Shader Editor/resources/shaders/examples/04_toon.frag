#version 330 core

in vec3 vNormalView;
in vec3 vViewPos;

uniform float time; // нужен только для закомментированного вращающегося света

out vec4 FragColor;

// Пример 4. Toon-шейдер (cel shading).
// Вместо плавного градиента освещение разбито на три ступени - эффект
// мультяшности. Тёмный контур по силуэту добавляется по значению rim:
// пиксели, смотрящие от камеры вскользь, окрашиваются почти в чёрный.
//
// Освещение выполняется в пространстве камеры (см. 04_toon.vert):
// камера всегда в точке (0,0,0) и смотрит вдоль -Z, поэтому
// lightDir = (x, y, +Z) всегда освещает видимую сторону модели.
//   ( 0.0,  0.0, 1.0) - свет строго из камеры (фронтальная вспышка)
//   ( 0.5,  0.4, 1.0) - сверху-справа
//   (-0.5, -0.4, 1.0) - снизу-слева

void main()
{
    vec3 normal = normalize(vNormalView);
    vec3 viewDir = normalize(-vViewPos);
    vec3 lightDir = normalize(vec3(0.4, 0.3, 1.0));

    // Для экспериментов: раскомментируйте - источник света будет медленно
    // облетать модель, и станет видно, как ступени освещения реагируют на свет
    // vec3 lightDir = normalize(vec3(sin(time) * 0.9, -0.4, cos(time) * 0.9));

    float diff = dot(normal, lightDir);
    float bands = floor(clamp(diff, 0.0, 1.0) * 3.0) / 3.0;

    vec3 baseColor = vec3(0.9, 0.45, 0.15);
    vec3 color = baseColor * (0.3 + 0.7 * bands);

    float rim = 1.0 - max(dot(normal, viewDir), 0.0);
    if (rim > 0.4)
        color = vec3(0.1, 0.05, 0.05);

    FragColor = vec4(color, 1.0);
}
