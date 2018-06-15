#include <iostream>
#include <string>
#include <GL\glew.h>
#include <GLFW/glfw3.h>
#include <thread> 

#include "imgui.h"
#include "imgui_impl_glfw_gl3.h"

#include "DrawingPanel.h"
#include "TextureData.h"
#include "ColourData.h"
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
void SetPixelValues(int startX, int width, int startY, int height, double xpos, double ypos, float brushScale, float brushOffset);
void drawLine(int x0, int y0, int x1, int y1);
void plotLineLow(int x0, int y0, int x1, int y1);
void plotLineHigh(int x0, int y0, int x1, int y1);
TextureData texData;
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
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		std::cout << "Open GL init error" << std::endl;
		return EXIT_FAILURE;
	}
	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

	// Setup Dear ImGui binding
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui_ImplGlfwGL3_Init(window, true);

	ImGui::StyleColorsDark();
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	DrawingPanel drawingPanel;
	drawingPanel.init(1.0f, 1.0f);

	TextureManager::getTextureDataFromFile("Resources\\goli.png", texData);

	std::cout << "\nComponent count = " << texData.getComponentCount();
	unsigned int texId = TextureManager::loadTextureFromData(texData, false);
	drawingPanel.setTextureID(texId);
	ShaderProgram shader;
	shader.compileShaders("Resources\\spriteBase.vs", "Resources\\spriteBase.fs");
	shader.linkShaders();
	int strengthValueUniform = shader.getUniformLocation("_HeightmapStrength");
	int normalMapModeOnUniform = shader.getUniformLocation("_normalMapModeOn");
	int modelMatrixUniform = shader.getUniformLocation("model");
	int widthUniform = shader.getUniformLocation("_HeightmapDimX");
	int heightUniform = shader.getUniformLocation("_HeightmapDimY");
	float strValue = 0.0f;
	float zoomLevel = 1;
	int normalMapMode = 3;
	float widthRes = 512;
	float heightRes = 512;
	glm::vec3 rotation = glm::vec3(0);
	Transform transform;
	transform.setPosition(0, 0);
	int k = 0;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	bool showHeightMapInput = true;
	bool isFullscreen = false;
	float brushScale = 0.01f;
	float brushOffset = 100.0f;
	double initTime = glfwGetTime();
	glm::vec2 prevMouseCoord = glm::vec2(-10, -10);
	while (!glfwWindowShouldClose(window))
	{
		double deltaTime = glfwGetTime() - initTime;
		glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);

		if (isKeyPressed(GLFW_KEY_1))
			normalMapMode = 1;
		if (isKeyPressed(GLFW_KEY_2))
			normalMapMode = 2;
		if (isKeyPressed(GLFW_KEY_3))
			normalMapMode = 3;

		if (isKeyPressed(GLFW_KEY_LEFT))
			transform.setX(transform.getPosition().x - 1 * deltaTime);
		if (isKeyPressed(GLFW_KEY_RIGHT))
			transform.setX(transform.getPosition().x + 1 * deltaTime);
		if (isKeyPressed(GLFW_KEY_UP))
			transform.setY(transform.getPosition().y + 1 * deltaTime);
		if (isKeyPressed(GLFW_KEY_DOWN))
			transform.setY(transform.getPosition().y - 1 * deltaTime);
		if (isKeyPressed(GLFW_KEY_8))
			transform.setRotation(transform.getRotation() + 1.0f * deltaTime);

		int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
		if (state == GLFW_PRESS)
		{
			double xpos, ypos;
			glfwGetCursorPos(window, &xpos, &ypos);
			glm::vec2 currentPos(xpos, ypos);
			if (currentPos != prevMouseCoord)
			{
				xpos = xpos / windowWidth;
				ypos = 1.0f - (ypos / (windowHeight - 150));

				float left = (-transform.getScale().x * 0.5f) + 0.5f;
				float right = (transform.getScale().x * 0.5f) + 0.5f;
				float top = (transform.getScale().y * 0.5f) + 0.5f;
				float bottom = (-transform.getScale().y * 0.5f) + 0.5f;

				if (xpos >= left && xpos <= right && ypos >= bottom && ypos <= top)
				{
					float prevX = prevMouseCoord.x / windowWidth;
					float prevY = 1.0f - (prevMouseCoord.y / (windowHeight - 150));

					if (prevMouseCoord != glm::vec2(-10, -10))
						drawLine(prevX * 511, prevY * 511, xpos * 511, ypos * 511);

					xpos = (xpos - left) / (right - left);
					ypos = (ypos - bottom) / (top - bottom);
					std::thread first(SetPixelValues, 0, 255, 0, 255, xpos, ypos, brushScale, brushOffset);
					std::thread second(SetPixelValues, 255, 512, 0, 255, xpos, ypos, brushScale, brushOffset);
					std::thread third(SetPixelValues, 0, 255, 255, 512, xpos, ypos, brushScale, brushOffset);
					std::thread fourth(SetPixelValues, 255, 512, 255, 512, xpos, ypos, brushScale, brushOffset);

					first.join();
					second.join();
					third.join();
					fourth.join();

					GLenum format = TextureManager::getTextureFormatFromData(4);
					glTexImage2D(GL_TEXTURE_2D, 0, format,
						texData.getWidth(), texData.getHeight(), 0, format, GL_UNSIGNED_BYTE, texData.getTextureData());
				}
				prevMouseCoord = currentPos;
			}
		}
		else
		{
			prevMouseCoord = glm::vec2(-10, -10);
		}

		if (isKeyPressed(GLFW_KEY_A))
			strValue += 2.5f * deltaTime;
		if (isKeyPressed(GLFW_KEY_D))
			strValue -= 2.5f * deltaTime;
		if (isKeyPressed(GLFW_KEY_W))
			zoomLevel += zoomLevel * 1.5f * deltaTime;
		if (isKeyPressed(GLFW_KEY_S))
			zoomLevel -= zoomLevel * 1.5f * deltaTime;
		zoomLevel = glm::clamp(zoomLevel, 0.1f, 5.0f);
		if (isKeyPressed(GLFW_KEY_F10))
		{
			if (saveScreenshot("D:\\scr.tga", windowWidth, windowHeight))
				std::cout << "Saved";
		}
		//---- Making sure the dimensions do not change for drawing panel ----//
		float aspectRatio = (float)windowWidth / (float)windowHeight;
		if (windowWidth < windowHeight)
			transform.setScale(glm::vec2(1, aspectRatio) * zoomLevel);
		else
			transform.setScale(glm::vec2(1.0f / aspectRatio, 1) * zoomLevel);
		transform.setX(glm::clamp(transform.getPosition().x, -0.5f, 0.9f));
		transform.setY(glm::clamp(transform.getPosition().y, -0.8f, 0.8f));
		transform.update();

		//---- Applying Shader Uniforms---//
		shader.applyShaderUniformMatrix(modelMatrixUniform, transform.getMatrix());
		shader.applyShaderFloat(strengthValueUniform, strValue);
		shader.applyShaderFloat(widthUniform, widthRes);
		shader.applyShaderFloat(heightUniform, heightRes);
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
		ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth() * 0.45f);
		if (ImGui::Button("Toggle Fullscreen", ImVec2(180, 40)))
		{
			if (!isFullscreen)
			{
				glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, mode->width, mode->height, 60);
			}
			else
				glfwSetWindowMonitor(window, NULL, 100, 100, (mode->width / 1.3f), (mode->height / 1.2f), 60);
			isFullscreen = !isFullscreen;
		}
		ImGui::Combo("Inputs Mode", &k, "All Inputs\0No Inputs\0RGB Input\0HSV Input\0HEX Input\0");
		if (ImGui::DragFloat("Normal Strength", &strValue, 0.1f, -100.0f, 100.0f, "X: %.2f")) {}
		if (ImGui::DragFloat("Horizontal Blur", &widthRes, 0.1f, -4028.0f, 4028.0f, "X: %.2f")) {}
		if (ImGui::DragFloat("Vertical Blur", &heightRes, 0.1f, -4028.0f, 4028.0f, "Y: %.2f")) {}
		if (ImGui::DragFloat("Brush Scale", &brushScale, 0.01f, -1.0f, 1.0f, "%.2f")) {}
		if (ImGui::DragFloat("Brush Offset", &brushOffset, 0.01f, -100.0f, 100.0f, "%.2f")) {}
		ImGui::PopItemWidth();
		ImGui::End();
		ImGui::PopStyleVar();
		ImGui::PopStyleVar();

		ImGui::Render();
		ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());

		initTime = glfwGetTime();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	ImGui_ImplGlfwGL3_Shutdown();
	ImGui::DestroyContext();
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
void drawLine(int x0, int y0, int x1, int y1)
{
	if (abs(y1 - y0) < abs(x1 - x0))
	{
		if (x0 > x1)
			plotLineLow(x1, y1, x0, y0);
		else
			plotLineLow(x0, y0, x1, y1);
	}
	else
	{
		if (y0 > y1)
			plotLineHigh(x1, y1, x0, y0);
		else
			plotLineHigh(x0, y0, x1, y1);
	}
}

void plotLineLow(int x0, int y0, int x1, int y1)
{
	int dx = x1 - x0;
	int	dy = y1 - y0;
	int	yi = 1;
	if (dy < 0)
	{
		yi = -1;
		dy = -dy;
	}
	int D = 2 * dy - dx;
	int y = y0;

	for (int x = x0; x < x1; x++)
	{
		texData.setTexelColor(255, 255, 255, 255, x, y);
		texData.setTexelColor(255, 255, 255, 255, x + 1, y);
		texData.setTexelColor(255, 255, 255, 255, x, y + 1);
		texData.setTexelColor(255, 255, 255, 255, x + 1, y + 1);
		if (D > 0)
		{
			y = y + yi;
			D = D - 2 * dx;
		}
		D = D + 2 * dy;
	}
}

void plotLineHigh(int x0, int y0, int x1, int y1)
{
	int dx = x1 - x0;
	int	dy = y1 - y0;
	int	xi = 1;
	if (dx < 0)
	{
		xi = -1;
		dx = -dx;
	}
	int D = 2 * dx - dy;
	int x = x0;

	for (int y = y0; y < y1; y++)
	{
		texData.setTexelColor(255, 255, 255, 255, x, y);
		texData.setTexelColor(255, 255, 255, 255, x + 1, y);
		texData.setTexelColor(255, 255, 255, 255, x, y + 1);
		texData.setTexelColor(255, 255, 255, 255, x + 1, y + 1);
		if (D > 0)
		{
			x = x + xi;
			D = D - 2 * dy;
		}
		D = D + 2 * dx;
	}
}
void SetPixelValues(int startX, int width, int startY, int height, double xpos, double ypos, float brushScale, float brushOffset)
{
	for (int i = startX; i < width; i++)
	{
		for (int j = startY; j < height; j++)
		{
			ColourData colData = texData.getTexelColor(i, j);
			unsigned char rVal = colData.getColour_8_Bit().r;
			float distance = glm::distance(glm::vec2(xpos, ypos), glm::vec2((double)i / 512.0f, (double)j / 512.0f));
			if (distance < brushScale)
			{
				distance = glm::pow(1.0f - (distance * brushOffset), 2);
				rVal = rVal + distance * (255.0f - rVal);
				texData.setTexelColor(rVal, rVal, rVal, 255, i, j);
			}
		}
	}
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