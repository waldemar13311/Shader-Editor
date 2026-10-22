# Shader Editor

Редактор GLSL-шейдеров с мгновенной компиляцией и 3D-предпросмотром.

Программа позволяет в реальном времени редактировать вершинный и фрагментный шейдеры графического конвейера OpenGL (профиль 3.3) и сразу видеть результат на модели:

- два текстовых редактора кода с подсветкой синтаксиса GLSL - вкладки *Vertex shader* и *Fragment shader*;
- моментальная перекомпиляция при каждом изменении кода, ошибки выводятся в консоль с указанием шейдера и строки;
- результат работы шейдеров отображается на 3D-модели формата STL, загружаемой через меню *File → Load model*;
- управление моделью мышью: вращение (левая кнопка), панорамирование (средняя кнопка), зум (колёсико);
- горячие клавиши редактора кода: undo/redo (`Ctrl/Cmd+Z`, `Ctrl/Cmd+Y`), комментирование выделенных строк (`Ctrl/Cmd+/`).

Окно разделено на две части: слева - 3D-визуализатор модели, справа - редакторы шейдеров и консоль компилятора.

## Примеры работы

![](gif%20examples/image1.gif)

![](gif%20examples/image2.gif)

![](gif%20examples/image3.gif)

![](gif%20examples/image4.gif)

![](gif%20examples/image5.gif)

## Примеры шейдеров

В каталоге `Shader Editor/resources/shaders/examples` лежат готовые примеры шейдеров (раскраска по нормалям, плазма, волны, toon-шейдинг, fresnel-свечение) с инструкцией по использованию и описанием доступных uniform-переменных.

## Сборка

Проект собирается через CMake. Зависимости GLFW и GLEW описаны в манифесте `vcpkg.json` и автоматически подтягиваются vcpkg. Остальные библиотеки (imgui, TextEditor, imgui_dock, glm, stb_image, tinyfiledialogs) входят в репозиторий и не требуют установки.

### Что нужно установить

- [CMake](https://cmake.org/download/) 3.20+
- [vcpkg](https://learn.microsoft.com/ru-ru/vcpkg/get_started/overview)
- компилятор с поддержкой C++17:
  - **Windows** - Visual Studio 2019 или новее;
  - **macOS** - Xcode Command Line Tools, плюс `brew install cmake pkg-config`;
  - **Linux (Debian/Ubuntu)** - `sudo apt install cmake pkg-config build-essential`.

### Сборка

```bash
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
cmake --build build
```

Если объявить переменную окружения `VCPKG_ROOT` (путь к установке vcpkg), команда сборки будет одинаковой на всех платформах; иначе подставьте свой путь к vcpkg вручную.

Альтернативно в Windows можно открыть `Shader Editor.sln` в Visual Studio и собрать решение напрямую.

### Запуск

Программа сама находит свои ресурсы - запускать можно из любого каталога:

```bash
./build/ShaderEditor                # macOS, Linux (Make/Ninja)
build/Debug/ShaderEditor.exe        # Windows (генератор Visual Studio)
```
