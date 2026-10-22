# Кроссплатформенная сборка Shader Editor.
#
# Зависимости GLFW и GLEW подтягиваются через vcpkg-манифест (vcpkg.json в корне):
#   cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=<путь к vcpkg>/scripts/buildsystems/vcpkg.cmake
#   cmake --build build
#
# imgui, TextEditor, imgui_dock, glm и stb_image остаются вендоренными
# (Shader Editor/Dependencies/include) - они header/source-only и не требуют сборки.
#
# Существующая сборка через Visual Studio (Shader Editor.sln) продолжает работать как раньше.
cmake_minimum_required(VERSION 3.20)
project(ShaderEditor C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(glfw3 CONFIG REQUIRED)
find_package(GLEW REQUIRED)

if(APPLE)
    find_package(OpenGL REQUIRED)
endif()

# Собственный код приложения (включая tinyfiledialogs.c)
file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS
    "Shader Editor/src/*.cpp"
    "Shader Editor/src/*.c"
)

# Вендоренные исходники imgui и аддонов к нему
set(IMGUI_DIR "Shader Editor/Dependencies/include/imgui")
set(IMGUI_SOURCES
    "${IMGUI_DIR}/imgui.cpp"
    "${IMGUI_DIR}/imgui_draw.cpp"
    "${IMGUI_DIR}/imgui_impl_glfw.cpp"
    "${IMGUI_DIR}/imgui_impl_opengl3.cpp"
    "${IMGUI_DIR}/imgui_dock.cpp"
    "${IMGUI_DIR}/TextEditor/TextEditor.cpp"
)

add_executable(ShaderEditor ${APP_SOURCES} ${IMGUI_SOURCES})

target_include_directories(ShaderEditor PRIVATE
    "Shader Editor/src"
    "Shader Editor/Dependencies/include"
)

target_link_libraries(ShaderEditor PRIVATE
    glfw
    GLEW::GLEW
)

if(APPLE)
    target_link_libraries(ShaderEditor PRIVATE OpenGL::GL)
    # OpenGL на macOS объявлен deprecated - глушим предупреждения
    target_compile_definitions(ShaderEditor PRIVATE GL_SILENCE_DEPRECATION)
endif()

# Чтобы в Visual Studio отладка запускалась с тем же рабочим каталогом, что и раньше
set_property(TARGET ShaderEditor PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/Shader Editor")

# Абсолютный путь к ресурсам: программа работает независимо от текущего каталога запуска
target_compile_definitions(ShaderEditor PRIVATE RESOURCE_ROOT="${CMAKE_SOURCE_DIR}/Shader Editor/resources/")
