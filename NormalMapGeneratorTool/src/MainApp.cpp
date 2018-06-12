#include <iostream>
#include <string>
#include <GL\glew.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw_gl3.h"

#include "DrawingPanel.h"
#include "TextureLoader.h"
#include "ShaderProgram.h"
#include "Transform.h"

int windowWidth = 1200;
int windowHeight = 1200;
GLFWwindow* window;
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
bool isKeyPressed(int key);
bool isKeyReleased(int key);
bool saveScreenshot(std::string filename, int w, int h);
int main(void)
{
	if (!glfwInit())
		return -1;
	window = glfwCreateWindow(windowWidth, windowHeight, "Normal Map Editor v0.1 alpha", NULL, NULL);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		std::cout << "Open GL init error" << std::endl;
		return EXIT_FAILURE;
	}
	// Setup Dear ImGui binding
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls
	ImGui_ImplGlfwGL3_Init(window, true);

	// Setup style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	DrawingPanel drawingPanel;
	drawingPanel.init(1.0f, 1.0f);
	unsigned int texId = TextureManager::loadTextureFromFile("Resources\\goli.png", "poke", false);
	drawingPanel.setTextureID(texId);
	ShaderProgram shader;
	shader.compileShaders("Resources\\spriteBase.vs", "Resources\\spriteBase.fs");
	shader.linkShaders();
	int strengthValueUniform = shader.getUniformLocation("_HeightmapStrength");
	int normalMapModeOnUniform = shader.getUniformLocation("_normalMapModeOn");
	int modelMatrixUniform = shader.getUniformLocation("model");
	float strValue = 0.0f;
	float zoomLevel = 0.0005f;
	int normalMapMode = 3;
	glm::vec3 rotation = glm::vec3(0);
	Transform transform;
	transform.setPosition(0, 0);
	int k = 0;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	bool showHeightMapInput = true;
	while (!glfwWindowShouldClose(window))
	{
		glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);

		if (isKeyPressed(GLFW_KEY_1))
			normalMapMode = 1;
		if (isKeyPressed(GLFW_KEY_2))
			normalMapMode = 2;
		if (isKeyPressed(GLFW_KEY_3))
			normalMapMode = 3;

		if (isKeyPressed(GLFW_KEY_LEFT))
			transform.setX(transform.getPosition().x - 0.005f);
		if (isKeyPressed(GLFW_KEY_RIGHT))
			transform.setX(transform.getPosition().x + 0.005f);
		if (isKeyPressed(GLFW_KEY_UP))
			transform.setY(transform.getPosition().y + 0.005f);
		if (isKeyPressed(GLFW_KEY_DOWN))
			transform.setY(transform.getPosition().y - 0.005f);
		if (isKeyPressed(GLFW_KEY_8))
			transform.setRotation(transform.getRotation() + 0.06f);

		if (isKeyPressed(GLFW_KEY_A))
			strValue += 0.05f;
		if (isKeyPressed(GLFW_KEY_D))
			strValue -= 0.05f;
		if (isKeyPressed(GLFW_KEY_W))
			zoomLevel += 0.000003f;
		if (isKeyPressed(GLFW_KEY_S))
			zoomLevel -= 0.000003f;
		zoomLevel = glm::clamp(zoomLevel, 0.0001f, 0.01f);
		if (isKeyPressed(GLFW_KEY_F10))
		{
			if (saveScreenshot("D:\\scr.tga", windowWidth, windowHeight))
				std::cout << "Saved";
		}

		//---- Making sure the dimensions do not change for drawing panel ----//
		float aspectRatio = (float)windowWidth / (float)windowHeight;
		transform.setScale(glm::vec2(windowWidth / aspectRatio, windowHeight * aspectRatio) * zoomLevel);
		transform.setX(glm::clamp(transform.getPosition().x, -0.3f, 0.3f));
		transform.setY(glm::clamp(transform.getPosition().y, -0.3f, 0.3f));
		transform.update();

		//---- Applying Shader Uniforms---//
		
		shader.applyShaderUniformMatrix(modelMatrixUniform, transform.getMatrix());
		shader.applyShaderFloat(strengthValueUniform, strValue);
		shader.applyShaderInt(normalMapModeOnUniform, normalMapMode);
		shader.use();
		drawingPanel.draw();
		
		ImGui_ImplGlfwGL3_NewFrame();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.95f);
		ImGuiWindowFlags window_flags = 0;
		window_flags |= ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoResize;
		window_flags |= ImGuiWindowFlags_NoCollapse;
		bool *p_open = NULL;
		
		ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiSetCond_Always);
		ImGui::SetNextWindowSize(ImVec2(glm::clamp(windowWidth * 0.15f, 250.0f, 600.0f), windowHeight), ImGuiSetCond_Always);
		ImGui::Begin("Settings", p_open, window_flags);
		ImGui::Combo("Inputs Mode", &k, "All Inputs\0No Inputs\0RGB Input\0HSV Input\0HEX Input\0");
		ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth() * 0.45f);
		if (ImGui::DragFloat("Normal Strength", &strValue, 0.1f, -100.0f, 100.0f, "X: %.3f")) {}
		ImGui::PopItemWidth();
		ImGui::End();
		ImGui::PopStyleVar();
		ImGui::PopStyleVar();


		ImGui::Render();
		ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	ImGui_ImplGlfwGL3_Shutdown();
	ImGui::DestroyContext();
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
bool isKeyPressed(int key)
{
	int state = glfwGetKey(window, key);
	if (state == GLFW_PRESS)
		return true;
	return false;
}
bool isKeyReleased(int key)
{
	int state = glfwGetKey(window, key);
	if (state == GLFW_RELEASE)
		return true;
	return false;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	windowWidth = width;
	windowHeight = height;
	glViewport(0, 0, width, height);
}

bool saveScreenshot(std::string filename, int w, int h)
{
	//This prevents the images getting padded 
	// when the width multiplied by 3 is not a multiple of 4
	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	int nSize = w * h * 3;
	// First let's create our buffer, 3 channels per Pixel
	char* dataBuffer = (char*)malloc(nSize * sizeof(char));

	if (!dataBuffer) return false;

	// Let's fetch them from the backbuffer	
	// We request the pixels in GL_BGR format, thanks to Berzeger for the tip
	glReadPixels((GLint)0, (GLint)0, (GLint)w, (GLint)h, GL_BGR, GL_UNSIGNED_BYTE, dataBuffer);

	//Now the file creation
#pragma warning(suppress : 4996)
	FILE *filePtr = fopen(filename.c_str(), "wb");
	if (!filePtr) return false;

	unsigned char TGAheader[12] = { 0,0,2,0,0,0,0,0,0,0,0,0 };
	unsigned char header[6] = { w % 256,w / 256, h % 256, h / 256, 24, 0 };
	// We write the headers
	fwrite(TGAheader, sizeof(unsigned char), 12, filePtr);
	fwrite(header, sizeof(unsigned char), 6, filePtr);
	// And finally our image data
	fwrite(dataBuffer, sizeof(GLubyte), nSize, filePtr);
	fclose(filePtr);

	free(dataBuffer);

	return true;
}