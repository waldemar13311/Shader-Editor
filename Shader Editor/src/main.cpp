#define STLLOADER_IMPLEMENTATION
#include "stlloader.h"

#include <iostream>

#include <GLEW/glew.h>
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


// Window size
static int WIDTH = 1200, HEIGHT = 600;

float CAMERA_X_POS = 0.0f;
float CAMERA_Y_POS = -1.0f;
float CAMERA_Z_POS = 0.0f;

static double XPOS = 0, YPOS = 0;
static float ROT_X = 0, ROT_Y = 0, ROT_Z = 0;
static double MAX_LENGTH;
static bool IS_LEFT_BUTTON_MOUSE_DOWN = false;
static bool IS_MIDDLE_BUTTON_MOUSE_DOWN = false;


void processInput(GLFWwindow *window);
void windowSizeCallback(GLFWwindow* window, int width, int height);
void scrollCallback(GLFWwindow* window, const double xoffset, const double yoffset);
static void cursorPosFun(GLFWwindow* window, double x, double y);
static void mouseButtonFun(GLFWwindow* window, int button, int action, int mods);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

glm::vec3 getOffsetVector(const glm::vec3& max, const glm::vec3& min);

int main()
{
	std::cout << "Starting GLFW context, OpenGL 3.3" << std::endl;

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

	const auto window = glfwCreateWindow(WIDTH, HEIGHT, "Shader Editor", nullptr, nullptr);
	if (window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwSetWindowSizeCallback(window, windowSizeCallback);
	glfwSetKeyCallback(window, keyCallback);
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


	glViewport(-WIDTH / 4, 0, WIDTH, HEIGHT);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_DEPTH_TEST);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	

	const auto pika = "../Shader Editor/resources/objects/pika.stl";
	const auto model = "../Shader Editor/resources/objects/1911_replica.stl";
	const auto bulbasaur = "../Shader Editor/resources/objects/bulbasaur.stl";
	const auto plane = "../Shader Editor/resources/objects/plane.stl";

	stlloader::Mesh scene;
	stlloader::parse_file(pika, scene);
	bool flag = true;


	static VertexArrayObject vao;


	auto vertexShaderCodeTemplate = FileReader::loadFromAsStr(
		"..\\Shader Editor\\resources\\shaders\\vert.txt");
	auto fragmentShaderCodeTemplate = FileReader::loadFromAsStr(
		"..\\Shader Editor\\resources\\shaders\\frag.txt");

	static TextEditor vertexShaderEditor;
	vertexShaderEditor.SetText(vertexShaderCodeTemplate);

	static TextEditor fragmentShaderEditor;
	fragmentShaderEditor.SetText(fragmentShaderCodeTemplate);




	MAX_LENGTH = scene.maxLength;
	const auto angleOfView = 45.0f;
	CAMERA_Y_POS = -(scene.lengthZ / (2.0f * glm::tan(glm::radians(angleOfView / 2.0f)))) - scene.lengthY;


	while (!glfwWindowShouldClose(window))
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		const auto guiSize = ImGui::GetIO().DisplaySize;


		static auto openLoadModelDialog = false;
		// Menu bar
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Load model"))
				{
					openLoadModelDialog = true;
				}
				ImGui::Separator();
				if (ImGui::MenuItem("Exit"))
				{
					break;
				}

				ImGui::EndMenu();
			}



			ImGui::EndMainMenuBar();
		}


		//		// Dock


		auto nowWight = guiSize.x;
		auto nowHeight = guiSize.y;


		const auto dockX = nowWight / 2.0;
		const auto dockY = 18;
		const auto dockWight = dockX;
		const auto dockHeight = nowHeight * 0.07;

		const auto textEditX = dockX;
		const auto textEditY = dockHeight + dockY;
		const auto textEditWight = dockWight;
		const auto textEditHeight = nowHeight * 0.6;


		ImGui::SetNextWindowPos(ImVec2(dockX, dockY));
		ImGui::SetNextWindowSize(ImVec2(dockWight, dockHeight));
		if (ImGui::Begin("Dock", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse))
		{
			ImGui::BeginDockspace();

			if (ImGui::BeginDock("Vertex shader"))
			{
				ImGui::SetNextWindowPos(ImVec2(textEditX, textEditY));
				ImGui::SetNextWindowSize(ImVec2(textEditWight, textEditHeight));
				ImGui::Begin("Vertex shader", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

				vertexShaderEditor.Render("VertShader");

				ImGui::End();
			}
			ImGui::EndDock();

			if (ImGui::BeginDock("Fragment shader"))
			{

				ImGui::SetNextWindowPos(ImVec2(textEditX, textEditY));
				ImGui::SetNextWindowSize(ImVec2(textEditWight, textEditHeight));
				ImGui::Begin("Fragment shader", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

				fragmentShaderEditor.Render("FragShader");

				ImGui::End();
			}
			ImGui::EndDock();

			ImGui::EndDockspace();
		}
		ImGui::End();

		const auto consoleX = dockX;
		const auto consoleY = textEditY + textEditHeight;
		const auto consoleWight = dockWight;
		const auto consoleHeight = nowHeight * 0.33 - dockY;

		std::string errorMessage;

	
		ShaderProgram shaderProgram;
		VertexShader vertexShader;
		vertexShader.setCode(vertexShaderEditor.GetText().c_str());
		bool isSuccessfullyLink = true;
		try
		{
			vertexShader.compile();
		}
		catch (...)
		{
			errorMessage = vertexShader.getInfoLog();
			isSuccessfullyLink = false;
		}

		FragmentShader fragmentShader;
		fragmentShader.setCode(fragmentShaderEditor.GetText().c_str());
		try
		{
			fragmentShader.compile();
		}
		catch (...)
		{
			errorMessage += fragmentShader.getInfoLog();
			isSuccessfullyLink = false;
		}

		shaderProgram.attachShader(vertexShader);
		shaderProgram.attachShader(fragmentShader);
		
		if (isSuccessfullyLink)
		{
			shaderProgram.link();
			errorMessage = "Compile successfully";
		}


		ImGui::SetNextWindowPos(ImVec2(consoleX, consoleY));
		ImGui::SetNextWindowSize(ImVec2(consoleWight, consoleHeight));
		ImGui::Begin("Console", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

		ImGui::Text(errorMessage.c_str());

		ImGui::End();




		if (flag)
		{

			vao.bind();

			VertexBufferObject positionsVbo;
			positionsVbo.bind();
			positionsVbo.bufferData(
				sizeof(float) * scene.facets.size() * 3 * 3,
				stlloader::getPositionsOfPtr(scene),
				GL_DYNAMIC_DRAW
			);
			positionsVbo.vertexAttribPointer(
				shaderProgram.getId(),
				"position",
				0, 3, 3 * sizeof(float),
				GL_FLOAT,
				GL_FALSE);
			positionsVbo.unBind();

			VertexBufferObject normalsVbo;
			normalsVbo.bind();
			normalsVbo.bufferData(
				sizeof(float) * scene.facets.size() * 3 * 3,
				stlloader::getNormalsOfPtr(scene),
				GL_DYNAMIC_DRAW
			);
			normalsVbo.vertexAttribPointer(
				shaderProgram.getId(),
				"normal",
				0, 3, 3 * sizeof(float),
				GL_FLOAT, GL_FALSE
			);
			normalsVbo.unBind();

			vao.unBind();
			flag = false;
		}


		vertexShader.deleteShader();
		fragmentShader.deleteShader();






		// Input
		// --
		processInput(window);

		// Render
		// --
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

		shaderProgram.use();

		glm::mat4 model(1.0f);

		model = glm::rotate(model, glm::radians(ROT_X), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(ROT_Z), glm::vec3(0.0f, 0.0f, 1.0f));

		auto vecOffsetToCenter = getOffsetVector(
			glm::vec3(scene.maxX, scene.maxY, scene.maxZ),
			glm::vec3(scene.minX, scene.minY, scene.minZ)
		);

		model = glm::translate(model, -vecOffsetToCenter);
		
		const glm::vec3 cameraPosition(CAMERA_X_POS, CAMERA_Y_POS, CAMERA_Z_POS);
		const glm::vec3 cameraDirection(CAMERA_X_POS, 0.0f, CAMERA_Z_POS);
		const glm::vec3 cameraDirectionUp(0.0f, 0.0f, 1.0f); 

		glm::mat4 view = glm::lookAt(cameraPosition, cameraDirection, cameraDirectionUp);
		
		glm::mat4 projection = glm::perspective(
			glm::radians(angleOfView), 
			static_cast<float>(WIDTH) / static_cast<float>(HEIGHT), 
			0.1f, 
			static_cast<float>(glm::abs(CAMERA_Y_POS) + MAX_LENGTH)
		);

		glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(model)));

		shaderProgram.setMat4("model", glm::value_ptr(model));
		shaderProgram.setMat4("view", glm::value_ptr(view));
		shaderProgram.setMat4("projection", glm::value_ptr(projection));
		shaderProgram.setMat3("normalMatrix", glm::value_ptr(normalMatrix));
		shaderProgram.setFloat("time", glfwGetTime());
		shaderProgram.setVec3("cameraPosition", CAMERA_X_POS, CAMERA_Y_POS, CAMERA_Z_POS);


		vao.bind();
		glDrawArrays(GL_TRIANGLES, 0, scene.facets.size() * 3 * 3);
		vao.unBind();

		shaderProgram.deleteProgram();

		// Rendering
		ImGui::Render();
		int display_w, display_h;
		glfwMakeContextCurrent(window);
		glfwGetFramebufferSize(window, &display_w, &display_h);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}

void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

void windowSizeCallback(GLFWwindow* window, int width, int height)
{
	WIDTH = width;
	HEIGHT = height;
	glViewport(-width / 4, 0, width, height);
}

void scrollCallback(GLFWwindow* window, const double xoffset, const double yoffset)
{
	yoffset < 0 ? CAMERA_Y_POS *= 1.1f : CAMERA_Y_POS /= 1.1f;
}

static void cursorPosFun(GLFWwindow* window, double x, double y)
{
	int wnd_width, wnd_height, fb_width, fb_height;
	double scale;

	glfwGetWindowSize(window, &wnd_width, &wnd_height);
	glfwGetFramebufferSize(window, &fb_width, &fb_height);

	if(x > wnd_width / 2.0)
		return;;

	scale = (double)fb_width / (double)wnd_width;

	x *= scale;
	y *= scale;

	if (IS_LEFT_BUTTON_MOUSE_DOWN)
	{
		ROT_X += static_cast<int>(y - YPOS);
		ROT_Z += static_cast<int>(x - XPOS);
	}

	if(IS_MIDDLE_BUTTON_MOUSE_DOWN)
	{
		CAMERA_X_POS += static_cast<float>((XPOS - x) / WIDTH * MAX_LENGTH);
		CAMERA_Z_POS += static_cast<float>((y - YPOS) / HEIGHT * MAX_LENGTH);
	}

	XPOS = x;
	YPOS = y;
}

static void mouseButtonFun(GLFWwindow* window, int button, int action, int mods)
{
	if ((button == GLFW_MOUSE_BUTTON_LEFT) && action == GLFW_PRESS)
	{
		IS_LEFT_BUTTON_MOUSE_DOWN = true;
	}
	else if (button == GLFW_MOUSE_BUTTON_LEFT)
	{
		IS_LEFT_BUTTON_MOUSE_DOWN = false;
	}

	if((button == GLFW_MOUSE_BUTTON_MIDDLE) && action == GLFW_PRESS)
	{
		IS_MIDDLE_BUTTON_MOUSE_DOWN = true;
	}
	else if ((button == GLFW_MOUSE_BUTTON_MIDDLE))
	{
		IS_MIDDLE_BUTTON_MOUSE_DOWN = false;
	}
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
	{

	}
}

glm::vec3 getOffsetVector(const glm::vec3& max, const glm::vec3& min)
{
	const auto lenXd2 = (max.x - min.x) / 2.0;
	const auto lenYd2 = (max.y - min.y) / 2.0;
	const auto lenZd2 = (max.z - min.z) / 2.0;

	return glm::vec3(max.x - lenXd2, max.y - lenYd2, max.z - lenZd2);
}
