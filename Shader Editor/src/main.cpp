#define STLLOADER_IMPLEMENTATION
#include "stlloader.h"

#include "tinyfiledialogs.h"

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "OpenGLOOP/Shaders/Shader.h"
#include "OpenGLOOP/Shaders/VertexShader.h"
#include "OpenGLOOP/Shaders/FragmentShader.h"
#include "OpenGLOOP/Shaders/ShaderProgram.h"
#include "FileReader/FileReader.h"

#include "OpenGLOOP/Buffers/VertexArrayObject.h"
#include "OpenGLOOP/Buffers/VertexBufferObject.h"
#include "OpenGLOOP/Buffers/ElementBufferObject.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/TextEditor/TextEditor.h"
#include "imgui/imgui_dock.h"

// Путь к каталогу ресурсов. Для CMake-сборки он задаётся в CMakeLists.txt абсолютным
// путём, поэтому программу можно запускать из любого каталога. Для сборки в Visual
// Studio используется относительный путь (как раньше - рабочий каталог "Shader Editor").
#ifndef RESOURCE_ROOT
#define RESOURCE_ROOT "../Shader Editor/resources/"
#endif

namespace
{
	// Параметры раскладки интерфейса
	constexpr float kConsoleHeightRatio = 0.30f;  // доля высоты окна под консоль
	constexpr float kZoomStep = 1.05f;            // шаг зума модели за одно деление колёсика
	constexpr float kFieldOfView = 45.0f;         // угол обзора камеры, градусы

	// Состояние вида/камеры. Одно место вместо россыпи глобальных переменных.
	struct ViewState
	{
		int width = 1200;
		int height = 600;

		float cameraX = 0.0f;
		float cameraY = -1.0f;
		float cameraZ = 0.0f;

		float rotX = 0.0f;
		float rotZ = 0.0f;

		double mouseX = 0.0;
		double mouseY = 0.0;
		float maxLength = 0.0f;

		bool leftButtonDown = false;
		bool middleButtonDown = false;
	};

	// GL-ресурсы геометрии модели. Живут в области видимости main(), пока контекст
	// OpenGL существует, - деструкторы корректно освобождают буферы.
	struct ModelResources
	{
		VertexArrayObject vao;
		VertexBufferObject positions;
		VertexBufferObject normals;
		bool uploaded = false; // буферы перезаливаются при каждой загрузке модели
	};

	// Кэш шейдерной программы: перекомпиляция только при изменении текста в редакторах
	struct ShaderCache
	{
		std::string vertexSource;
		std::string fragmentSource;
		std::unique_ptr<ShaderProgram> program; // последняя успешно слинкованная программа
		std::string error;                      // сообщение для консоли
	};

	// Состояние интерфейса редактора
	struct GuiState
	{
		TextEditor vertexEditor;
		TextEditor fragmentEditor;
		std::string loadError; // сообщение о неудачной загрузке модели
	};

	// Всё состояние приложения, передаваемое функциям отрисовки
	struct AppContext
	{
		stlloader::Mesh scene;
		ModelResources model;
		ShaderCache shaders;
		GuiState gui;
	};
}

static ViewState gView;
static AppContext* gApp = nullptr;   // для колбэков GLFW (ресайз окна)
static ImFont* gEditorFont = nullptr; // моноширинный шрифт редакторов кода

void windowSizeCallback(GLFWwindow* window, int width, int height);
void scrollCallback(GLFWwindow* window, const double xoffset, const double yoffset);
static void cursorPosFun(GLFWwindow* window, double x, double y);
static void mouseButtonFun(GLFWwindow* window, int button, int action, int mods);

glm::vec3 getOffsetVector(const glm::vec3& max, const glm::vec3& min);

void renderFrame(GLFWwindow* window, AppContext& app);
void drawMainMenu(GLFWwindow* window, AppContext& app);
void drawShaderEditors(AppContext& app);
void drawConsole(AppContext& app, const std::string& compileMessage);
void updateShaderProgram(AppContext& app);
void uploadGeometry(AppContext& app);
void renderScene(GLFWwindow* window, AppContext& app);
void processInput(GLFWwindow* window);
float fitCameraDistance(const stlloader::Mesh& scene);

int main()
{
	std::cout << "Starting GLFW context, OpenGL 3.3" << std::endl;

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

	const auto window = glfwCreateWindow(gView.width, gView.height, "Shader Editor", nullptr, nullptr);
	if (window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwSetWindowSizeCallback(window, windowSizeCallback);
	glfwSetScrollCallback(window, scrollCallback);
	glfwSetCursorPosCallback(window, cursorPosFun);
	glfwSetMouseButtonCallback(window, mouseButtonFun);
	glfwSwapInterval(1);
	glfwMakeContextCurrent(window);

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		return -1;
	}

	const char* glsl_version = "#version 330 core";
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	// На Retina-экранах фреймбуфер крупнее логического размера окна.
	// Вместо растягивания встроенного битмап-шрифта (получается размыто) загружаем
	// настоящий TTF-шрифт сразу в физическом размере - текст и виджеты чёткие.
	{
		int fbWidth, fbHeight;
		glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
		const auto contentScale = static_cast<float>(fbWidth) / static_cast<float>(gView.width);
		if (contentScale > 1.0f)
		{
			const char* uiFontPaths[] =
			{
#ifdef __APPLE__
				"/System/Library/Fonts/SFNS.ttf",
				"/System/Library/Fonts/Helvetica.ttc",
				"/System/Library/Fonts/Supplemental/Arial.ttf",
#elif defined(_WIN32)
				"C:\\Windows\\Fonts\\segoeui.ttf",
				"C:\\Windows\\Fonts\\arial.ttf",
#else
				"/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
#endif
			};
			ImFont* uiFont = nullptr;
			for (const auto path : uiFontPaths)
			{
				uiFont = io.Fonts->AddFontFromFileTTF(path, 14.0f * contentScale);
				if (uiFont != nullptr)
					break;
			}

			// Отдельный моноширинный шрифт для редакторов кода
			static ImFont* editorFont = nullptr;
			const char* monoFontPaths[] =
			{
#ifdef __APPLE__
				"/System/Library/Fonts/Menlo.ttc",
				"/System/Library/Fonts/SFNSMono.ttf",
				"/System/Library/Fonts/Supplemental/Courier New.ttf",
#elif defined(_WIN32)
				"C:\\Windows\\Fonts\\consola.ttf",
				"C:\\Windows\\Fonts\\cour.ttf",
#else
				"/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
#endif
			};
			for (const auto path : monoFontPaths)
			{
				gEditorFont = io.Fonts->AddFontFromFileTTF(path, 11.0f * contentScale);
				if (gEditorFont != nullptr)
					break;
			}

			if (uiFont == nullptr)
			{
				// Ни один шрифт не загрузился - возвращаемся к растягиванию встроенного
				io.FontGlobalScale = contentScale;
			}

			std::cout << "UI font: " << (uiFont != nullptr ? "loaded" : "fallback (ProggyClean)")
				<< ", editor font: " << (gEditorFont != nullptr ? "loaded" : "fallback (UI font)")
				<< ", contentScale: " << contentScale << std::endl;
		}
	}

	glEnable(GL_MULTISAMPLE);
	glEnable(GL_DEPTH_TEST);

	// Загрузка стартовой модели и шаблонов шейдеров
	AppContext app;
	{
		const auto pika = std::string(RESOURCE_ROOT) + "objects/pika.stl";
		stlloader::parse_file(pika.c_str(), app.scene);

		app.gui.vertexEditor.SetText(FileReader::loadFromAsStr(RESOURCE_ROOT "shaders/vert.txt"));
		app.gui.fragmentEditor.SetText(FileReader::loadFromAsStr(RESOURCE_ROOT "shaders/frag.txt"));
	}
	gView.maxLength = app.scene.maxLength;
	gView.cameraY = fitCameraDistance(app.scene);

	// Ресурсы GL должны быть уничтожены до разрушения контекста - потому в этой области
	{
		gApp = &app;
		while (!glfwWindowShouldClose(window))
		{
			renderFrame(window, app);
			glfwPollEvents();
		}
		gApp = nullptr;
	}

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}

float fitCameraDistance(const stlloader::Mesh& scene)
{
	return -(scene.lengthZ / (2.0f * glm::tan(glm::radians(kFieldOfView / 2.0f)))) - scene.lengthY;
}

// Вся отрисовка одного кадра. Вынесена из main, чтобы кадр можно было перерисовать
// и из колбэка изменения размера окна: на macOS во время перетаскивания рамки окна
// главный цикл не выполняется, и без этого картинка «замирает» до отпускания мыши.
void renderFrame(GLFWwindow* window, AppContext& app)
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	drawMainMenu(window, app);
	drawShaderEditors(app);

	processInput(window);
	updateShaderProgram(app);
	uploadGeometry(app);
	renderScene(window, app);

	drawConsole(app, app.shaders.error);

	// Rendering
	ImGui::Render();
	glfwMakeContextCurrent(window);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	glfwSwapBuffers(window);
}

void drawMainMenu(GLFWwindow* window, AppContext& app)
{
	if (!ImGui::BeginMainMenuBar())
		return;

	if (ImGui::BeginMenu("File"))
	{
		if (ImGui::MenuItem("Load model"))
		{
			// Нативный диалог выбора файла (macOS/Windows/Linux)
			const char* filterPatterns[] = { "*.stl" };
			const char* filePath = tinyfd_openFileDialog(
				"Load 3D model", "",
				1, filterPatterns, "STL models (*.stl)", 0);

			if (filePath != nullptr)
			{
				try
				{
					// parse_file дописывает фейсы в конец mesh - старую модель надо очистить
					app.scene = stlloader::Mesh();
					stlloader::parse_file(filePath, app.scene);

					// пересчитать камеру под новую модель и сбросить вид
					gView.maxLength = app.scene.maxLength;
					gView.cameraY = fitCameraDistance(app.scene);
					gView.cameraX = 0.0f;
					gView.cameraZ = 0.0f;
					gView.rotX = 0.0f;
					gView.rotZ = 0.0f;

					app.model.uploaded = false; // перезалить буферы с новой геометрией
					app.gui.loadError.clear();
				}
				catch (const std::exception& e)
				{
					app.gui.loadError = std::string("Failed to load model: ") + e.what();
				}
			}
		}
		ImGui::Separator();
		if (ImGui::MenuItem("Exit"))
		{
			glfwSetWindowShouldClose(window, GLFW_TRUE);
		}

		ImGui::EndMenu();
	}

	ImGui::EndMainMenuBar();
}

void drawShaderEditors(AppContext& app)
{
	// Правая половина окна: сверху - док-панели с редакторами шейдеров, снизу - консоль.
	// Все размеры вычисляются от фактического размера окна и высоты главного меню,
	// поэтому раскладка корректно подстраивается при изменении размера окна и масштаба интерфейса.
	const auto guiSize = ImGui::GetIO().DisplaySize;
	const auto rightX = guiSize.x / 2.0f;
	const auto rightWidth = guiSize.x - rightX;
	const auto top = ImGui::GetFrameHeight();
	const auto consoleHeight = guiSize.y * kConsoleHeightRatio;
	const auto editorsHeight = guiSize.y - top - consoleHeight;

	ImGui::SetNextWindowPos(ImVec2(rightX, top));
	ImGui::SetNextWindowSize(ImVec2(rightWidth, editorsHeight));
	if (ImGui::Begin("Dock", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse))
	{
		ImGui::BeginDockspace();

		// Моноширинный шрифт редакторов кода (загружается в main)
		ImFont* const font = gEditorFont != nullptr ? gEditorFont : ImGui::GetFont();

		if (ImGui::BeginDock("Vertex shader"))
		{
			ImGui::PushFont(font);
			app.gui.vertexEditor.Render("VertShader");
			ImGui::PopFont();
		}
		ImGui::EndDock();

		if (ImGui::BeginDock("Fragment shader"))
		{
			ImGui::PushFont(font);
			app.gui.fragmentEditor.Render("FragShader");
			ImGui::PopFont();
		}
		ImGui::EndDock();

		ImGui::EndDockspace();
	}
	ImGui::End();
}

void drawConsole(AppContext& app, const std::string& compileMessage)
{
	const auto guiSize = ImGui::GetIO().DisplaySize;
	const auto rightX = guiSize.x / 2.0f;
	const auto rightWidth = guiSize.x - rightX;
	const auto top = ImGui::GetFrameHeight();
	const auto consoleHeight = guiSize.y * kConsoleHeightRatio;
	const auto editorsHeight = guiSize.y - top - consoleHeight;

	ImGui::SetNextWindowPos(ImVec2(rightX, top + editorsHeight));
	ImGui::SetNextWindowSize(ImVec2(rightWidth, consoleHeight));
	ImGui::Begin("Console", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

	ImGui::TextUnformatted(compileMessage.c_str());
	if (!app.gui.loadError.empty())
		ImGui::TextUnformatted(app.gui.loadError.c_str());

	ImGui::End();
}

// Перекомпилирует шейдерную программу только если текст в редакторах изменился.
// При ошибке компиляции продолжает работать последняя успешная программа.
void updateShaderProgram(AppContext& app)
{
	const auto vertexSource = app.gui.vertexEditor.GetText();
	const auto fragmentSource = app.gui.fragmentEditor.GetText();

	if (vertexSource == app.shaders.vertexSource && fragmentSource == app.shaders.fragmentSource)
		return;

	app.shaders.vertexSource = vertexSource;
	app.shaders.fragmentSource = fragmentSource;

	auto newProgram = std::make_unique<ShaderProgram>();
	std::string error;

	try
	{
		VertexShader vertexShader;
		vertexShader.setCode(vertexSource.c_str());
		vertexShader.compile();

		FragmentShader fragmentShader;
		fragmentShader.setCode(fragmentSource.c_str());
		fragmentShader.compile();

		newProgram->attachShader(vertexShader);
		newProgram->attachShader(fragmentShader);
		newProgram->link();

		app.shaders.error = "Compile successfully";
		app.shaders.program = std::move(newProgram);
	}
	catch (const std::exception&)
	{
		// логи обеих стадий компиляции (пустой лог = не было ошибок на этой стадии)
		VertexShader probe;
		probe.setCode(vertexSource.c_str());
		error += probe.getInfoLog();

		FragmentShader probeFragment;
		probeFragment.setCode(fragmentSource.c_str());
		error += probeFragment.getInfoLog();

		app.shaders.error = error.empty() ? "Failed to compile shaders" : error;
		// последняя рабочая программа сохраняется в app.shaders.program
	}
}

// Перезаливает геометрию модели в GPU-буферы (при первой отрисовке и после загрузки новой модели)
void uploadGeometry(AppContext& app)
{
	if (app.model.uploaded || app.shaders.program == nullptr)
		return;

	const auto programId = app.shaders.program->getId();
	const auto vertexCount = static_cast<GLsizeiptr>(app.scene.facets.size() * 3 * 3);

	app.model.vao.bind();

	app.model.positions.bind();
	app.model.positions.bufferData(
		sizeof(float) * vertexCount,
		stlloader::getPositionsOfPtr(app.scene),
		GL_DYNAMIC_DRAW
	);
	app.model.positions.vertexAttribPointer(
		programId,
		"position",
		0, 3, 3 * sizeof(float),
		GL_FLOAT,
		GL_FALSE);
	app.model.positions.unBind();

	app.model.normals.bind();
	app.model.normals.bufferData(
		sizeof(float) * vertexCount,
		stlloader::getNormalsOfPtr(app.scene),
		GL_DYNAMIC_DRAW
	);
	app.model.normals.vertexAttribPointer(
		programId,
		"normal",
		0, 3, 3 * sizeof(float),
		GL_FLOAT, GL_FALSE
	);
	app.model.normals.unBind();

	app.model.vao.unBind();
	app.model.uploaded = true;
}

void renderScene(GLFWwindow* window, AppContext& app)
{
	// Viewport задаём каждый кадр от фактического размера фреймбуфера: сдвиг влево
	// на четверть, чтобы 3D-вид занимал левую половину окна, пока справа редакторы.
	// Так раскладка не ломается при разворачивании окна и смене Retina-масштаба.
	int fbWidth, fbHeight;
	glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
	glViewport(-fbWidth / 4, 0, fbWidth, fbHeight);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

	if (app.shaders.program == nullptr || app.model.uploaded == false)
		return;

	const auto& scene = app.scene;
	app.shaders.program->use();

	glm::mat4 model(1.0f);
	model = glm::rotate(model, glm::radians(gView.rotX), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(gView.rotZ), glm::vec3(0.0f, 0.0f, 1.0f));

	const auto vecOffsetToCenter = getOffsetVector(
		glm::vec3(scene.maxX, scene.maxY, scene.maxZ),
		glm::vec3(scene.minX, scene.minY, scene.minZ)
	);
	model = glm::translate(model, -vecOffsetToCenter);

	const glm::vec3 cameraPosition(gView.cameraX, gView.cameraY, gView.cameraZ);
	const glm::vec3 cameraDirection(gView.cameraX, 0.0f, gView.cameraZ);
	const glm::vec3 cameraDirectionUp(0.0f, 0.0f, 1.0f);

	const glm::mat4 view = glm::lookAt(cameraPosition, cameraDirection, cameraDirectionUp);
	const glm::mat4 projection = glm::perspective(
		glm::radians(kFieldOfView),
		static_cast<float>(gView.width) / static_cast<float>(gView.height),
		0.1f,
		static_cast<float>(glm::abs(gView.cameraY) + gView.maxLength)
	);
	const glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(model)));

	app.shaders.program->setMat4("model", glm::value_ptr(model));
	app.shaders.program->setMat4("view", glm::value_ptr(view));
	app.shaders.program->setMat4("projection", glm::value_ptr(projection));
	app.shaders.program->setMat3("normalMatrix", glm::value_ptr(normalMatrix));
	app.shaders.program->setFloat("time", static_cast<float>(glfwGetTime()));
	app.shaders.program->setFloat("meshSize", gView.maxLength);
	app.shaders.program->setVec3("cameraPosition", gView.cameraX, gView.cameraY, gView.cameraZ);

	// В буфере facets * 3 вершин (по 3 координаты на каждую) -
	// рисуем ровно столько вершин, сколько загружено
	app.model.vao.bind();
	glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(scene.facets.size() * 3));
	app.model.vao.unBind();
}

void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

void windowSizeCallback(GLFWwindow* window, int width, int height)
{
	gView.width = width;
	gView.height = height;

	// На macOS во время перетаскивания рамки окна главный цикл не выполняется -
	// перерисовываем кадр прямо отсюда, иначе картинка замирает до отпускания мыши
	if (gApp != nullptr)
		renderFrame(window, *gApp);
}

void scrollCallback(GLFWwindow* window, const double xoffset, const double yoffset)
{
	// Бэкенд imgui в этой версии не подписан на колёсико (подписка закомментирована
	// в imgui_impl_glfw.cpp) - передаём событие ему вручную, иначе редакторы не прокручиваются
	ImGui::GetIO().MouseWheelH += static_cast<float>(xoffset);
	ImGui::GetIO().MouseWheel += static_cast<float>(yoffset);

	// Если курсор над правой половиной окна (редакторы кода, консоль) -
	// прокрутка должна листать текст интерфейса, а не масштабировать модель
	double cursorX, cursorY;
	glfwGetCursorPos(window, &cursorX, &cursorY);
	if (cursorX > gView.width / 2.0)
		return;

	yoffset < 0 ? gView.cameraY *= kZoomStep : gView.cameraY /= kZoomStep;
}

static void cursorPosFun(GLFWwindow* window, double x, double y)
{
	int wnd_width, wnd_height, fb_width, fb_height;
	double scale;

	glfwGetWindowSize(window, &wnd_width, &wnd_height);
	glfwGetFramebufferSize(window, &fb_width, &fb_height);

	if (x > wnd_width / 2.0)
		return;

	scale = (double)fb_width / (double)wnd_width;

	x *= scale;
	y *= scale;

	if (gView.leftButtonDown)
	{
		gView.rotX += static_cast<int>(y - gView.mouseY);
		gView.rotZ += static_cast<int>(x - gView.mouseX);
	}

	if (gView.middleButtonDown)
	{
		gView.cameraX += static_cast<float>((gView.mouseX - x) / gView.width * gView.maxLength);
		gView.cameraZ += static_cast<float>((y - gView.mouseY) / gView.height * gView.maxLength);
	}

	gView.mouseX = x;
	gView.mouseY = y;
}

static void mouseButtonFun(GLFWwindow* window, int button, int action, int mods)
{
	if ((button == GLFW_MOUSE_BUTTON_LEFT) && action == GLFW_PRESS)
	{
		gView.leftButtonDown = true;
	}
	else if (button == GLFW_MOUSE_BUTTON_LEFT)
	{
		gView.leftButtonDown = false;
	}

	if ((button == GLFW_MOUSE_BUTTON_MIDDLE) && action == GLFW_PRESS)
	{
		gView.middleButtonDown = true;
	}
	else if (button == GLFW_MOUSE_BUTTON_MIDDLE)
	{
		gView.middleButtonDown = false;
	}
}

glm::vec3 getOffsetVector(const glm::vec3& max, const glm::vec3& min)
{
	const auto lenXd2 = (max.x - min.x) / 2.0;
	const auto lenYd2 = (max.y - min.y) / 2.0;
	const auto lenZd2 = (max.z - min.z) / 2.0;

	return glm::vec3(max.x - lenXd2, max.y - lenYd2, max.z - lenZd2);
}
