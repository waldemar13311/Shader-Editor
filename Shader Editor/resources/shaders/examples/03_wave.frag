#version 330 core

in float vHeight;
in vec3 vNormal;

out vec4 FragColor;

// Пример 3. Волны (фрагментная часть).
// Цвет по высоте волны: впадины - глубокий синий, гребни - светло-голубой.
// Плюс простое диффузное освещение, чтобы рельеф читался.

void main()
{
    // vHeight - величина смещения волны (примерно ±0.1): масштабируем в 0..1
    float t = clamp(vHeight * 5.0 + 0.5, 0.0, 1.0);
    vec3 trough = vec3(0.05, 0.15, 0.45);
    vec3 crest  = vec3(0.55, 0.85, 1.0);

    vec3 color = mix(trough, crest, t);

    vec3 lightDir = normalize(vec3(0.4, -0.7, 0.6));
    float diff = max(dot(normalize(vNormal), lightDir), 0.0);

    FragColor = vec4(color * (0.4 + 0.6 * diff), 1.0);
}
