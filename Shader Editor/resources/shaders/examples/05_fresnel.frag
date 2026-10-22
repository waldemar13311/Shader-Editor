#version 330 core

in vec3 vNormal;
in vec3 vViewPos;

out vec4 FragColor;

// Пример 5. Fresnel-свечение.
// По эффекту Френеля грани, которые видны вскользь (по силуэту модели),
// начинают светиться неоновым цветом. Степень 2.5 управляет резкостью
// перехода: больше степень - уже кайма. Попробуйте поменять glowColor.

void main()
{
    vec3 normal = normalize(vNormal);
    vec3 viewDir = normalize(-vViewPos);

    float fresnel = pow(1.0 - max(dot(normal, viewDir), 0.0), 2.5);

    vec3 baseColor = vec3(0.08, 0.08, 0.12);
    vec3 glowColor = vec3(0.2, 0.9, 1.0);

    vec3 color = mix(baseColor, glowColor, fresnel) + glowColor * fresnel * 0.5;

    FragColor = vec4(color, 1.0);
}
