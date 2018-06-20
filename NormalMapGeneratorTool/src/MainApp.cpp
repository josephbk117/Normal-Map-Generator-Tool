#include <iostream>
#include <string>
#include <GL\glew.h>
#include <GLFW/glfw3.h>
#include <thread> 

#include "imgui.h"
#include "imgui_impl_glfw_gl3.h"

#include "DrawingPanel.h"
#include "FrameBuffer.h"
#include "TextureData.h"
#include "ColourData.h"
#include "TextureLoader.h"
#include "ShaderProgram.h"
#include "Transform.h"

//TODO:Mouse drag (middle mouse) for moving and scrolling for zooming, Button to switch view mode 

int windowWidth = 1200;
int windowHeight = 1200;
unsigned int framebuffer;
unsigned int textureColorbuffer;
GLFWwindow* window;
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void CustomColourImGuiTheme(ImGuiStyle* dst = (ImGuiStyle*)0);
bool isKeyPressed(int key);
bool isKeyReleased(int key);
bool saveScreenshot(std::string filename, int xOff, int yOff, int w, int h);
void SetPixelValues(int startX, int width, int startY, int height, double xpos, double ypos, float brushScale, float brushOffset, bool heightMapPositiveDir);
void drawLine(int x0, int y0, int x1, int y1);
void plotLineLow(int x0, int y0, int x1, int y1);
void plotLineHigh(int x0, int y0, int x1, int y1);
TextureData texData;
int main(void)
{
	if (!glfwInit())
		return -1;
	window = glfwCreateWindow(windowWidth, windowHeight, "Normal Map Editor v0.1 alpha", NULL, NULL);
	glfwGetFramebufferSize(window, &windowWidth, &windowHeight);
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
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	CustomColourImGuiTheme();
	ImFont* font = io.Fonts->AddFontFromFileTTF("Resources\\arial.ttf", 16.0f);
	IM_ASSERT(font != NULL);

	DrawingPanel normalmapPanel;
	normalmapPanel.init(1.0f, 1.0f);
	DrawingPanel frameDrawingPanel;
	frameDrawingPanel.init(1.0f, 1.0f);

	TextureManager::getTextureDataFromFile("Resources\\goli.png", texData);

	unsigned int texId = TextureManager::loadTextureFromData(texData, false);
	normalmapPanel.setTextureID(texId);

	ShaderProgram normalmapShader;
	normalmapShader.compileShaders("Resources\\spriteBase.vs", "Resources\\spriteBase.fs");
	normalmapShader.linkShaders();

	ShaderProgram frameShader;
	frameShader.compileShaders("Resources\\spriteBase.vs", "Resources\\frameBuffer.fs");
	frameShader.linkShaders();


	int frameModelMatrixUniform = normalmapShader.getUniformLocation("model");
	int modelMatrixUniform = normalmapShader.getUniformLocation("model");
	int strengthValueUniform = normalmapShader.getUniformLocation("_HeightmapStrength");
	int normalMapModeOnUniform = normalmapShader.getUniformLocation("_normalMapModeOn");
	int widthUniform = normalmapShader.getUniformLocation("_HeightmapDimX");
	int heightUniform = normalmapShader.getUniformLocation("_HeightmapDimY");
	int specularityUniform = normalmapShader.getUniformLocation("_Specularity");
	int lightIntensityUniform = normalmapShader.getUniformLocation("_LightIntensity");
	float strValue = 0.0f;
	float specularity = 0;
	float lightIntensity = 1;
	float zoomLevel = 1;
	int mapViewMode = 3;
	float widthRes = texData.getWidth();
	float heightRes = texData.getHeight();
	bool heightMapPositiveDir = false;
	glm::vec3 rotation = glm::vec3(0);
	int k = 0;
	bool showHeightMapInput = true;
	bool isFullscreen = false;
	float brushScale = 0.01f;
	float brushOffset = 1.0f;
	double initTime = glfwGetTime();
	glm::vec2 prevMouseCoord = glm::vec2(-10, -10);

	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	glGenTextures(1, &textureColorbuffer);
	glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowWidth, windowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	while (!glfwWindowShouldClose(window))
	{
		double deltaTime = glfwGetTime() - initTime;
		initTime = glfwGetTime();

		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glEnable(GL_DEPTH_TEST);
		glClearColor(64.0f / 255.0f, 75.0f / 255.0f, 105.0f / 255.0f, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glBindTexture(GL_TEXTURE_2D, texId);
		normalmapShader.use();

		if (isKeyPressed(GLFW_KEY_J))
			mapViewMode = 1;
		if (isKeyPressed(GLFW_KEY_K))
			mapViewMode = 2;
		if (isKeyPressed(GLFW_KEY_L))
			mapViewMode = 3;

		if (isKeyPressed(GLFW_KEY_LEFT))
			normalmapPanel.getTransform()->translate(-1 * deltaTime, 0);
		if (isKeyPressed(GLFW_KEY_RIGHT))
			normalmapPanel.getTransform()->translate(1 * deltaTime, 0);
		if (isKeyPressed(GLFW_KEY_UP))
			normalmapPanel.getTransform()->translate(0, 1 * deltaTime);
		if (isKeyPressed(GLFW_KEY_DOWN))
			normalmapPanel.getTransform()->translate(0, -1 * deltaTime);
		if (isKeyPressed(GLFW_KEY_8))
			normalmapPanel.getTransform()->rotate(1.0f * deltaTime);

		int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
		if (state == GLFW_PRESS)
		{
			double xpos, ypos;
			glfwGetCursorPos(window, &xpos, &ypos);
			glm::vec2 currentPos(xpos, ypos);
			if (currentPos != prevMouseCoord)
			{
				xpos = xpos / windowWidth;
				ypos = 1.0f - (ypos / windowHeight);
				if (normalmapPanel.isPointInPanel(xpos, ypos))
				{
					float prevX = prevMouseCoord.x / windowWidth;
					float prevY = 1.0f - (prevMouseCoord.y / windowHeight);

					glm::vec4 worldDimensions = normalmapPanel.getPanelWorldDimension();
					xpos = (xpos - worldDimensions.x) / (worldDimensions.y - worldDimensions.x);
					ypos = (ypos - worldDimensions.w) / (worldDimensions.z - worldDimensions.w);

					int left = glm::clamp((int)((xpos - brushScale) * 512.0f), 0, 512);
					int right = glm::clamp((int)((xpos + brushScale) * 512.0f), 0, 512);
					int bottom = glm::clamp((int)((ypos - brushScale) * 512.0f), 0, 512);
					int top = glm::clamp((int)((ypos + brushScale) * 512.0f), 0, 512);

					std::thread first(SetPixelValues, left, right, bottom, top, xpos, ypos, brushScale, brushOffset, heightMapPositiveDir);
					first.join();

					GLenum format = TextureManager::getTextureFormatFromData(4);
					glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texData.getWidth(),
						texData.getHeight(), format, GL_UNSIGNED_BYTE, texData.getTextureData());
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

		//---- Making sure the dimensions do not change for drawing panel ----//
		float aspectRatio = (float)windowWidth / (float)windowHeight;
		if (windowWidth < windowHeight)
			normalmapPanel.getTransform()->setScale(glm::vec2(1, aspectRatio) * zoomLevel);
		else
			normalmapPanel.getTransform()->setScale(glm::vec2(1.0f / aspectRatio, 1) * zoomLevel);
		normalmapPanel.getTransform()->setX(glm::clamp(normalmapPanel.getTransform()->getPosition().x, -0.5f, 0.9f));
		normalmapPanel.getTransform()->setY(glm::clamp(normalmapPanel.getTransform()->getPosition().y, -0.8f, 0.8f));
		normalmapPanel.getTransform()->update();
		//---- Applying Shader Uniforms---//
		normalmapShader.applyShaderUniformMatrix(modelMatrixUniform, normalmapPanel.getTransform()->getMatrix());
		normalmapShader.applyShaderFloat(strengthValueUniform, strValue);
		normalmapShader.applyShaderFloat(specularityUniform, specularity);
		normalmapShader.applyShaderFloat(lightIntensityUniform, lightIntensity);
		normalmapShader.applyShaderFloat(widthUniform, widthRes);
		normalmapShader.applyShaderFloat(heightUniform, heightRes);
		normalmapShader.applyShaderInt(normalMapModeOnUniform, mapViewMode);
		normalmapShader.use();
		normalmapPanel.draw();

		if (isKeyPressed(GLFW_KEY_F10))
		{
			int widthSub = windowWidth - (int)(normalmapPanel.getPanelWorldDimension().y * windowWidth);
			int heightSub = windowHeight - (int)(normalmapPanel.getPanelWorldDimension().z * windowHeight);
			if (saveScreenshot("D:\\scr.tga", widthSub, heightSub, windowWidth - (2 * widthSub), windowHeight - (2 * heightSub)))
				std::cout << "Saved";
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDisable(GL_DEPTH_TEST);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		frameShader.use();
		frameShader.applyShaderUniformMatrix(modelMatrixUniform, glm::mat4());
		glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
		frameDrawingPanel.setTextureID(textureColorbuffer);
		frameDrawingPanel.draw();

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
		ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth() * 0.5f);
		if (ImGui::Button("Toggle Fullscreen", ImVec2(ImGui::GetContentRegionAvailWidth(), 40)))
		{
			if (!isFullscreen)
				glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, mode->width, mode->height, 60);
			else
				glfwSetWindowMonitor(window, NULL, 100, 100, (mode->width / 1.3f), (mode->height / 1.2f), 60);
			isFullscreen = !isFullscreen;
		}
		if (ImGui::Button("Toggle Height", ImVec2(ImGui::GetContentRegionAvailWidth(), 40)))
		{
			heightMapPositiveDir = !heightMapPositiveDir;
		}
		ImGui::Spacing();
		ImGui::Text("VIEW MODE");
		ImGui::Separator();
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 10));
		int modeButtonWidth = (int)(ImGui::GetContentRegionAvailWidth() / 3.0f);
		ImGui::Spacing();
		if (ImGui::Button("Height", ImVec2(modeButtonWidth - 5, 40))) { mapViewMode = 3; }
		ImGui::SameLine(0, 5);
		if (ImGui::Button("Normal", ImVec2(modeButtonWidth - 5, 40))) { mapViewMode = 1; }
		ImGui::SameLine(0, 5);
		if (ImGui::Button("3D Plane", ImVec2(modeButtonWidth, 40))) { mapViewMode = 2; }
		ImGui::PopStyleVar();
		ImGui::Text("PROPERTIES");
		ImGui::Separator();
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 10));
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 5));
		ImGui::Spacing();
		if (ImGui::DragFloat(" Brush Scale", &brushScale, 0.001f, 0.0f, 1.0f, "%.3f")) {}
		if (ImGui::DragFloat(" Brush Offset", &brushOffset, 0.01f, 0.0f, 100.0f, "%.2f")) {}
		if (ImGui::DragFloat(" Normal Strength", &strValue, 0.1f, -100.0f, 100.0f, "%.2f")) {}
		if (ImGui::DragFloat(" Light Intensity", &lightIntensity, 0.01f, 0.0f, 1.0f, "%.2f")) {}
		if (ImGui::DragFloat(" Specularity", &specularity, 0.01f, 0.0f, 1.0f, "%.2f")) {}

		ImGui::PopStyleVar();
		ImGui::PopStyleVar();
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
		/*texData.setTexelColor(255, 255, 255, 255, x, y);
		texData.setTexelColor(255, 255, 255, 255, x + 1, y);
		texData.setTexelColor(255, 255, 255, 255, x, y + 1);
		texData.setTexelColor(255, 255, 255, 255, x + 1, y + 1);*/
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
		/*texData.setTexelColor(255, 255, 255, 255, x, y);
		texData.setTexelColor(255, 255, 255, 255, x + 1, y);
		texData.setTexelColor(255, 255, 255, 255, x, y + 1);
		texData.setTexelColor(255, 255, 255, 255, x + 1, y + 1);*/
		if (D > 0)
		{
			x = x + xi;
			D = D - 2 * dy;
		}
		D = D + 2 * dx;
	}
}

void SetPixelValues(int startX, int width, int startY, int height, double xpos, double ypos, float brushScale, float brushOffset, bool heightMapPositiveDir)
{
	ColourData colData;
	float rVal;
	float distance;
	glm::vec2 pixelPos(xpos, ypos);
	float px_width = texData.getWidth();
	float px_height = texData.getHeight();
	for (int i = startX; i < width; i++)
	{
		for (int j = startY; j < height; j++)
		{
			colData = texData.getTexelColor(i, j);
			rVal = colData.getColourIn_0_1_Range().r;
			distance = glm::distance(pixelPos, glm::vec2((double)i / px_width, (double)j / px_height));
			if (distance < brushScale)
			{
				distance = glm::clamp((1.0f - (distance / brushScale)) * brushOffset, 0.0f, 1.0f);
				rVal = rVal + distance * ((heightMapPositiveDir ? 1.0f : 0.0f) - rVal);
				ColourData col(rVal, rVal, rVal, 1.0f);
				texData.setTexelColor(col, i, j);
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
	glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, windowWidth, windowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glViewport(0, 0, width, height);
}

bool saveScreenshot(std::string filename, int xOff, int yOff, int w, int h)
{
	//This prevents the images getting padded 
	// when the width multiplied by 3 is not a multiple of 4
	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	int nSize = w * h * 3;
	// First let's create our buffer, 3 channels per Pixel
	char* dataBuffer = (char*)malloc(nSize * sizeof(char));

	if (!dataBuffer) return false;
	glReadPixels((GLint)xOff, (GLint)yOff, (GLint)w, (GLint)h, GL_BGR, GL_UNSIGNED_BYTE, dataBuffer);
	//glGetTexImage(GL_TEXTURE_2D, 0, GL_BGR, GL_UNSIGNED_BYTE, dataBuffer);

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
void CustomColourImGuiTheme(ImGuiStyle* dst)
{
	ImGuiStyle* style = dst ? dst : &ImGui::GetStyle();
	ImVec4* colors = style->Colors;

	const ImVec4 PRIMARY_COL = ImVec4(40 / 255.0f, 49 / 255.0f, 73.0f / 255.0f, 1.1f);
	const ImVec4 SECONDARY_COL = ImVec4(247 / 255.0f, 56 / 255.0f, 89 / 255.0f, 1.1f);
	const ImVec4 ACCENT_COL = ImVec4(64.0f / 255.0f, 75.0f / 255.0f, 105.0f / 255.0f, 1.1f);
	const ImVec4 WHITE = ImVec4(255 / 255.0f, 247 / 255.0f, 240 / 255.0f, 1.1f);
	const ImVec4 DARK_GREY = ImVec4(40 / 255.0f, 40 / 255.0f, 40 / 255.0f, 1.1f);

	colors[ImGuiCol_Text] = WHITE;
	colors[ImGuiCol_TextDisabled] = DARK_GREY;
	colors[ImGuiCol_WindowBg] = PRIMARY_COL;
	colors[ImGuiCol_ChildBg] = DARK_GREY;
	colors[ImGuiCol_PopupBg] = DARK_GREY;
	colors[ImGuiCol_Border] = ACCENT_COL;
	colors[ImGuiCol_BorderShadow] = DARK_GREY;
	colors[ImGuiCol_FrameBg] = ACCENT_COL;
	colors[ImGuiCol_FrameBgHovered] = PRIMARY_COL;
	colors[ImGuiCol_FrameBgActive] = SECONDARY_COL;
	colors[ImGuiCol_TitleBg] = ACCENT_COL;
	colors[ImGuiCol_TitleBgActive] = SECONDARY_COL;
	colors[ImGuiCol_TitleBgCollapsed] = ACCENT_COL;
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.26f, 0.0f, 0.26f, 1.00f);
	colors[ImGuiCol_ScrollbarBg] = ACCENT_COL;
	colors[ImGuiCol_ScrollbarGrab] = SECONDARY_COL;
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.59f, 0.49f, 0.49f, 0.80f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.59f, 0.59f, 0.49f, 1.00f);
	colors[ImGuiCol_CheckMark] = SECONDARY_COL;
	colors[ImGuiCol_SliderGrab] = ACCENT_COL;
	colors[ImGuiCol_SliderGrabActive] = ACCENT_COL;
	colors[ImGuiCol_Button] = SECONDARY_COL;
	colors[ImGuiCol_ButtonHovered] = PRIMARY_COL;
	colors[ImGuiCol_ButtonActive] = ACCENT_COL;
	colors[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_Separator] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.14f, 0.44f, 0.80f, 0.78f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.14f, 0.44f, 0.80f, 1.00f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.30f, 0.30f, 0.70f, 0.46f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
	colors[ImGuiCol_PlotLines] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.45f, 0.00f, 1.00f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.0f, 0.0f, 0.20f, 0.45f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
}