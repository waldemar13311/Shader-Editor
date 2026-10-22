#version 330 core

in vec3 vPosition;
in vec3 vNormalView;
in vec3 vViewPos;

uniform float time;
uniform float meshSize; // габарит модели — программа подставляет его сама

out vec4 FragColor;

// Пример 6. Электрические молнии.
// Тёмно-синяя модель, по поверхности которой бегут тонкие ярко-голубые
// разряды, периодически вспыхивая. Узор строится из процедурного шума:
// молния — это изолиния значения шума (уровень, «пережатый» в тонкую
// линию функцией pow), а движение и вспышки задаются временем.
//
// Что покрутить:
//   множитель p * K     — масштаб ячеек узора (мельче/крупнее)
//   скорости в offset   — скорость «бегания» молний
//   степени pow(...)    — толщина и резкость разрядов (больше — тоньше)
//   base / boltColor    — цвета корпуса и разрядов

float hash(vec3 p)
{
    p = fract(p * 0.3183099 + vec3(0.1, 0.2, 0.3));
    p *= 17.0;
    return fract(p.x * p.y * p.z * (p.x + p.y + p.z));
}

// плавный value noise по решётке
float noise(vec3 x)
{
    vec3 i = floor(x);
    vec3 f = fract(x);
    f = f * f * (3.0 - 2.0 * f);

    return mix(mix(mix(hash(i + vec3(0.0, 0.0, 0.0)), hash(i + vec3(1.0, 0.0, 0.0)), f.x),
                   mix(hash(i + vec3(0.0, 1.0, 0.0)), hash(i + vec3(1.0, 1.0, 0.0)), f.x), f.y),
               mix(mix(hash(i + vec3(0.0, 0.0, 1.0)), hash(i + vec3(1.0, 0.0, 1.0)), f.x),
                   mix(hash(i + vec3(0.0, 1.0, 1.0)), hash(i + vec3(1.0, 1.0, 1.0)), f.x), f.y),
               f.z);
}

// несколько октав шума — узор становится «рваным», как настоящий разряд
float fbm(vec3 p)
{
    float v = 0.0;
    float a = 0.5;
    for (int i = 0; i < 4; ++i)
    {
        v += a * noise(p);
        p *= 2.0;
        a *= 0.5;
    }
    return v;
}

void main()
{
    // нормализуем координаты по габариту модели: узор одинаковый
    // и на маленькой, и на большой модели (STL часто в миллиметрах)
    vec3 p = vPosition / max(meshSize, 0.001);

    // шум, медленно движущийся во времени — молнии «живут»
    vec3 flow = vec3(time * 0.25, time * 0.15, -time * 0.2);
    float n = fbm(p * 1.2 + flow);

    // два слоя разрядов: редкие изолинии шума, пережатые в толстые линии
    float bolt1 = pow(1.0 - abs(fract(n * 3.0) - 0.5) * 2.0, 7.0);
    float bolt2 = pow(1.0 - abs(fract(n * 5.0 + 0.37) - 0.5) * 2.0, 12.0);

    // вспышки: редкие импульсы яркости, раскошенные по фазе шума
    float flash = smoothstep(0.75, 1.0, sin(time * 1.5 + n * 6.0) * 0.5 + 0.5);

    vec3 base = vec3(0.02, 0.06, 0.25);
    vec3 boltColor = vec3(0.5, 0.95, 1.0);

    vec3 color = base + boltColor * (bolt1 * 0.8 + bolt2 * 1.2) * (0.4 + flash);

    // лёгкая подсветка силуэта, чтобы модель читалась на фоне
    vec3 normal = normalize(vNormalView);
    vec3 viewDir = normalize(-vViewPos);
    float fresnel = pow(1.0 - max(dot(normal, viewDir), 0.0), 3.0);
    color += boltColor * fresnel * 0.25;

    FragColor = vec4(color, 1.0);
}
