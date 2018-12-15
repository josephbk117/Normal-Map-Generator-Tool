#include <iostream>
#include <string>
#include <GL\glew.h>
#include <GLFW/glfw3.h>
#include <thread> 

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl2.h"

#include "Camera.h"
#include "DrawingPanel.h"
#include "FrameBufferSystem.h"
#include "TextureData.h"
#include "ColourData.h"
#include "BrushData.h"
#include "TextureLoader.h"
#include "ShaderProgram.h"
#include "Transform.h"
#include "WindowTransformUtility.h"
#include "FileExplorer.h"
#include "ModelObject.h"

#include "stb_image_write.h"
#include "MeshLoadingSystem.h"

//TODO : Drawing should take copy of entire image before button press and make changes on that.(prevents overwrite)
//TODO : Add detail value in normal settings, Sampling rate in shader
//TODO : MAke camera move around instead of model
//TODO : Implement modal dialouges
//TODO : Rotation editor values
//TODO : Distance based drawing
//TODO : Undo/Redo Capability, 20 steps in RAM after that Write to disk

const ImVec4 PRIMARY_COL = ImVec4(40 / 255.0f, 49 / 255.0f, 73.0f / 255.0f, 1.1f);
const ImVec4 TITLE_COL = ImVec4(30 / 255.0f, 39 / 255.0f, 63.0f / 255.0f, 1.1f);
const ImVec4 SECONDARY_COL = ImVec4(247 / 255.0f, 56 / 255.0f, 89 / 255.0f, 1.1f);
const ImVec4 ACCENT_COL = ImVec4(64.0f / 255.0f, 75.0f / 255.0f, 105.0f / 255.0f, 1.1f);
const ImVec4 WHITE = ImVec4(1, 1, 1, 1.1f);
const ImVec4 DARK_GREY = ImVec4(40 / 255.0f, 40 / 255.0f, 40 / 255.0f, 1.1f);

const std::string MODELS_PATH = "Resources\\3D Models\\Primitives\\";
const std::string CUBE_MODEL_PATH = MODELS_PATH + "Cube.obj";
const std::string CYLINDER_MODEL_PATH = MODELS_PATH + "Cylinder.obj";
const std::string SPHERE_MODEL_PATH = MODELS_PATH + "Sphere.obj";
const std::string TORUS_MODEL_PATH = MODELS_PATH + "Torus.obj";

const int WINDOW_SIZE_MIN = 640;

int windowWidth = 1600; //Temporary Hack, Max resoltion will not face issues
int windowHeight = 800;
int maxWindowWidth = -1;
int maxWindowHeight = -1;

FrameBufferSystem fbs;
FrameBufferSystem previewFbs;
GLFWwindow* window;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void CustomColourImGuiTheme(ImGuiStyle* dst = (ImGuiStyle*)0);
bool isKeyPressed(int key);
bool isKeyReleased(int key);
bool exportImage(const std::string& filename, int xOff, int yOff, int w, int h);
void SetPixelValues(TextureData& texData, int startX, int width, int startY, int height, double xpos, double ypos, const BrushData& brushData);
void SetBrushPixelValues(TextureData& inputTexData, int startX, int width, int startY, int height, double xpos, double ypos, const BrushData& brushData);
void SetBluredPixelValues(TextureData& inputTexData, int startX, int width, int startY, int height, double xpos, double ypos, const BrushData& brushData);
void HandleMiddleMouseButtonInput(int state, glm::vec2 &prevMiddleMouseButtonCoord, double deltaTime, DrawingPanel &normalmapPanel);
void HandleLeftMouseButtonInput_UI(int state, glm::vec2 &initPos, WindowSide &windowSideAtInitPos, double x, double y, bool &isMaximized, glm::vec2 &prevGlobalFirstMouseCoord);
void HandleLeftMouseButtonInput_NormalMapInteraction(int state, glm::vec2 &prevMouseCoord, BrushData &brushData, DrawingPanel &normalmapPanel, bool isBlurOn);
void SaveNormalMapToFile(char  saveLocation[500]);
void WindowTopBarDisplay(unsigned int minimizeTexture, unsigned int restoreTexture, bool &isMaximized, unsigned int closeTexture);
inline void DisplayPreview(bool * p_open, const ImGuiWindowFlags &window_flags, int &modelViewMode, glm::vec3 &diffuseColour, glm::vec3 &ambientColour, glm::vec3 &lightColour);
inline void DisplayLightSettingsUserInterface(float &lightIntensity, float &specularity, float &specularityStrength, glm::vec3 &lightDirection);
inline void DisplayNormalSettingsUserInterface(float &normalMapStrength, bool &flipX_Ydir, bool &redChannelActive, bool &greenChannelActive, bool &blueChannelActive);
inline void DisplayBrushSettingsUserInterface(bool &isBlurOn, BrushData &brushData);
inline void HandleKeyboardInput(float &normalMapStrength, double deltaTime, int &mapDrawViewMode, DrawingPanel &frameDrawingPanel, bool &isMaximized);
inline void BottomBarDisplay(bool * p_open, const ImGuiWindowFlags &window_flags);
inline void SideBarDisplay(bool * p_open, const ImGuiWindowFlags &window_flags, bool &isFullscreen, const GLFWvidmode * mode, DrawingPanel &frameDrawingPanel, char  imageLoadLocation[500], FileExplorer &fileExplorer, std::string &path, bool &updateImageLocation, char  saveLocation[500], bool &shouldSaveNormalMap, bool &changeSize, int &mapDrawViewMode);
void SetStatesForSavingNormalMap(bool &changeSize, glm::vec2 &prevWindowSize, int &retflag);
void SetupImGui();

float zoomLevel = 1;
float modelPreviewRotationSpeed = 0.1f;
float modelPreviewZoomLevel = -5.0f;
bool isUsingCustomTheme = false;

TextureData texData;
MeshLoadingSystem::MeshLoader modelLoader;
ModelObject *modelPreviewObj = nullptr;

int main(void)
{
	if (!glfwInit())
		return -1;
	glfwWindowHint(GLFW_DECORATED, false);
	window = glfwCreateWindow(windowWidth, windowHeight, "Nora Normal Map Editor v0.8 alpha", NULL, NULL);

	const GLFWvidmode * mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	maxWindowWidth = mode->width;
	maxWindowHeight = mode->height;
	glfwSetWindowSizeLimits(window, WINDOW_SIZE_MIN, WINDOW_SIZE_MIN, maxWindowWidth, maxWindowHeight);
	glfwGetFramebufferSize(window, &windowWidth, &windowHeight);
	glfwSetWindowPos(window, 0, 0);

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
	SetupImGui();
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetScrollCallback(window, scroll_callback);

	modelPreviewObj = modelLoader.CreateModelFromFile(CUBE_MODEL_PATH); // Default loaded model in preview window

	DrawingPanel normalmapPanel;
	normalmapPanel.init(1.0f, 1.0f);
	DrawingPanel frameDrawingPanel;
	frameDrawingPanel.init(1.0f, 1.0f);
	DrawingPanel previewFrameDrawingPanel;
	previewFrameDrawingPanel.init(1.0f, 1.0f);
	DrawingPanel brushPanel;
	brushPanel.init(1.0f, 1.0f);

	unsigned int closeTextureId = TextureManager::loadTextureFromFile("Resources\\UI\\closeIcon.png", "close", false);
	unsigned int restoreTextureId = TextureManager::loadTextureFromFile("Resources\\UI\\maxWinIcon.png", "restore", false);
	unsigned int minimizeTextureId = TextureManager::loadTextureFromFile("Resources\\UI\\toTrayIcon.png", "mini", false);
	unsigned int logoTextureId = TextureManager::loadTextureFromFile("Resources\\UI\\icon.png", "mdini", false);
	unsigned int penguinTextureId = TextureManager::loadTextureFromFile("Resources\\Penguins.jpg", "penguin", false);

	TextureManager::getTextureDataFromFile("Resources\\goli.png", texData);
	unsigned int heightmapTexId = TextureManager::loadTextureFromData(texData, false);
	normalmapPanel.setTextureID(heightmapTexId);

	ShaderProgram normalmapShader;
	normalmapShader.compileShaders("Resources\\spriteBase.vs", "Resources\\spriteBase.fs");
	normalmapShader.linkShaders();

	ShaderProgram modelViewShader;
	modelViewShader.compileShaders("Resources\\modelView.vs", "Resources\\modelView.fs");
	modelViewShader.linkShaders();

	ShaderProgram frameShader;
	frameShader.compileShaders("Resources\\spriteBase.vs", "Resources\\frameBuffer.fs");
	frameShader.linkShaders();

	ShaderProgram brushPreviewShader;
	brushPreviewShader.compileShaders("Resources\\spriteBase.vs", "Resources\\brushPreview.fs");
	brushPreviewShader.linkShaders();

	Camera camera;
	camera.init(windowWidth, windowHeight);

	int frameModelMatrixUniform = normalmapShader.getUniformLocation("model");
	int normalPanelModelMatrixUniform = normalmapShader.getUniformLocation("model");
	int strengthValueUniform = normalmapShader.getUniformLocation("_HeightmapStrength");
	int normalMapModeOnUniform = normalmapShader.getUniformLocation("_normalMapModeOn");
	int widthUniform = normalmapShader.getUniformLocation("_HeightmapDimX");
	int heightUniform = normalmapShader.getUniformLocation("_HeightmapDimY");
	int specularityUniform = normalmapShader.getUniformLocation("_Specularity");
	int specularityStrengthUniform = normalmapShader.getUniformLocation("_SpecularStrength");
	int lightIntensityUniform = normalmapShader.getUniformLocation("_LightIntensity");
	int flipXYdirUniform = normalmapShader.getUniformLocation("_flipX_Ydir");
	int RedChannelUniform = normalmapShader.getUniformLocation("_Channel_R");
	int GreenChannelUniform = normalmapShader.getUniformLocation("_Channel_G");
	int BlueChannelUniform = normalmapShader.getUniformLocation("_Channel_B");
	int lightDirectionUniform = normalmapShader.getUniformLocation("lightDir");

	int brushPreviewModelUniform = brushPreviewShader.getUniformLocation("model");
	int brushPreviewOffsetUniform = brushPreviewShader.getUniformLocation("_BrushOffset");
	int brushPreviewStrengthUniform = brushPreviewShader.getUniformLocation("_BrushStrength");
	int brushPreviewColourUniform = brushPreviewShader.getUniformLocation("_BrushColour");

	int modelViewmodelUniform = modelViewShader.getUniformLocation("model");
	int modelVieviewUniform = modelViewShader.getUniformLocation("view");
	int modelViewprojectionUniform = modelViewShader.getUniformLocation("projection");
	int modelNormalMapModeUniform = modelViewShader.getUniformLocation("_normalMapModeOn");
	int modelNormalMapStrengthUniform = modelViewShader.getUniformLocation("_HeightmapStrength");
	int modelLightIntensityUniform = modelViewShader.getUniformLocation("_LightIntensity");
	int modelLightSpecularityUniform = modelViewShader.getUniformLocation("_Specularity");
	int modelLightSpecularityStrengthUniform = modelViewShader.getUniformLocation("_SpecularStrength");
	int modelLightDirectionUniform = modelViewShader.getUniformLocation("lightDir");
	int modelLightColourUniform = modelViewShader.getUniformLocation("lightColour");
	int modelDiffuseColourUniform = modelViewShader.getUniformLocation("diffuseColour");
	int modelAmbientColourUniform = modelViewShader.getUniformLocation("ambientColour");
	int modelHeightMapTextureUniform = modelViewShader.getUniformLocation("inTexture");
	int modelTextureMapTextureUniform = modelViewShader.getUniformLocation("inTexture2");

	float normalMapStrength = 10.0f;
	float specularity = 10.0f;
	float specularityStrength = 0.5f;
	float lightIntensity = 0.5f;
	glm::vec3 lightDirection = glm::vec3(90.0f, 90.0f, 60.0f);
	zoomLevel = 1;
	int mapDrawViewMode = 1;
	int modelViewMode = 1;
	const float widthRes = texData.getWidth();
	const float heightRes = texData.getHeight();
	bool flipX_Ydir = false;
	bool redChannelActive = true;
	bool greenChannelActive = true;
	bool blueChannelActive = true;

	glm::vec3 rotation = glm::vec3(0);
	int k = 0;
	bool showHeightMapInput = true;
	bool isFullscreen = false;
	bool isMaximized = false;
	bool isBlurOn = false;

	BrushData brushData;
	brushData.brushScale = 10.0f;
	brushData.brushOffset = 1.0f;
	brushData.brushStrength = 1.0f;
	brushData.brushMinHeight = 0.0f;
	brushData.brushMaxHeight = 1.0f;
	brushData.brushRate = 0.0f;
	brushData.heightMapPositiveDir = false;
	double initTime = glfwGetTime();

	fbs.init(windowWidth, windowHeight);
	previewFbs.init(windowWidth, windowHeight);

	glm::vec2 prevMouseCoord = glm::vec2(-10, -10);
	glm::vec2 prevMiddleMouseButtonCoord = glm::vec2(-10, -10);
	glm::vec2 prevGlobalFirstMouseCoord = glm::vec2(-500, -500);

	std::string path;
	std::string prevPath;
	FileExplorer fileExplorer;
	bool updateImageLocation = false;
	bool shouldSaveNormalMap = false;
	bool changeSize = false;
	glm::vec2  prevWindowSize = glm::vec2(500, 500);


	while (!glfwWindowShouldClose(window))
	{
		double deltaTime = glfwGetTime() - initTime;
		initTime = glfwGetTime();

		glViewport(0, 0, windowWidth, windowHeight);
		if (shouldSaveNormalMap)
		{
			int retflag;
			SetStatesForSavingNormalMap(changeSize, prevWindowSize, retflag);
			if (retflag == 3) continue;
		}
		static glm::vec2 initPos = glm::vec2(-1000, -1000);
		static WindowSide windowSideAtInitPos = WindowSide::NONE;
		double x, y;
		glfwGetCursorPos(window, &x, &y);
		glm::vec2 mouseCoord = glm::vec2(x, y);
		HandleKeyboardInput(normalMapStrength, deltaTime, mapDrawViewMode, frameDrawingPanel, isMaximized);

		fbs.BindFrameBuffer();
		glClearColor(0.9f, 0.5f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		glBindTexture(GL_TEXTURE_2D, heightmapTexId);
		normalmapShader.use();

		WindowSide currentMouseCoordWindowSide = WindowTransformUtility::GetWindowSideAtMouseCoord((int)x, (int)y, windowWidth, windowHeight);
		if (windowSideAtInitPos == WindowSide::LEFT || windowSideAtInitPos == WindowSide::RIGHT || currentMouseCoordWindowSide == WindowSide::LEFT || currentMouseCoordWindowSide == WindowSide::RIGHT)
			ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
		else if (windowSideAtInitPos == WindowSide::TOP || windowSideAtInitPos == WindowSide::BOTTOM || currentMouseCoordWindowSide == WindowSide::TOP || currentMouseCoordWindowSide == WindowSide::BOTTOM)
			ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
		else
			ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNWSE);

		//---- Making sure the dimensions do not change for drawing panel ----//
		float aspectRatio = (float)windowWidth / (float)windowHeight;
		glm::vec2 aspectRatioHolder;
		if (windowWidth < windowHeight)
			aspectRatioHolder = glm::vec2(1, aspectRatio);
		else
			aspectRatioHolder = glm::vec2(1.0f / aspectRatio, 1);
		frameDrawingPanel.getTransform()->setScale(aspectRatioHolder * zoomLevel);
		frameDrawingPanel.getTransform()->setX(glm::clamp(frameDrawingPanel.getTransform()->getPosition().x, -0.5f, 0.5f));
		frameDrawingPanel.getTransform()->setY(glm::clamp(frameDrawingPanel.getTransform()->getPosition().y, -1.0f, 1.0f));
		frameDrawingPanel.getTransform()->update();

		int leftMouseButtonState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
		int middleMouseButtonState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE);

		HandleLeftMouseButtonInput_UI(leftMouseButtonState, initPos, windowSideAtInitPos, x, y, isMaximized, prevGlobalFirstMouseCoord);
		HandleLeftMouseButtonInput_NormalMapInteraction(leftMouseButtonState, prevMouseCoord, brushData, frameDrawingPanel, isBlurOn);
		HandleMiddleMouseButtonInput(middleMouseButtonState, prevMiddleMouseButtonCoord, deltaTime, frameDrawingPanel);

		/*if (isKeyPressed(GLFW_KEY_C))
		{
			normalmapPanel.getTransform()->setX(normalmapPanel.getTransform()->getPosition().x + 0.01f);
		}
		else if (isKeyPressed(GLFW_KEY_X))
		{
			normalmapPanel.getTransform()->setX(normalmapPanel.getTransform()->getPosition().x - 0.01f);
		}
		else if (isKeyPressed(GLFW_KEY_F))
		{
			normalmapPanel.getTransform()->setY(normalmapPanel.getTransform()->getPosition().y - 0.01f);
		}
		else if (isKeyPressed(GLFW_KEY_G))
		{
			normalmapPanel.getTransform()->setY(normalmapPanel.getTransform()->getPosition().y + 0.01f);
		}*/
		normalmapPanel.getTransform()->update();
		//---- Applying Normal Map Shader Uniforms---//
		normalmapShader.applyShaderUniformMatrix(normalPanelModelMatrixUniform, normalmapPanel.getTransform()->getMatrix());
		normalmapShader.applyShaderFloat(strengthValueUniform, normalMapStrength);
		normalmapShader.applyShaderFloat(specularityUniform, specularity);
		normalmapShader.applyShaderFloat(specularityStrengthUniform, specularityStrength);
		normalmapShader.applyShaderFloat(lightIntensityUniform, lightIntensity);
		normalmapShader.applyShaderVector3(lightDirectionUniform, glm::normalize(glm::vec3(lightDirection.x / 180.0f, lightDirection.y / 180.0f, lightDirection.z / 180.0f)));
		normalmapShader.applyShaderFloat(widthUniform, widthRes);
		normalmapShader.applyShaderFloat(heightUniform, heightRes);
		normalmapShader.applyShaderInt(normalMapModeOnUniform, mapDrawViewMode);
		normalmapShader.applyShaderBool(flipXYdirUniform, flipX_Ydir);
		normalmapShader.applyShaderBool(RedChannelUniform, redChannelActive);
		normalmapShader.applyShaderBool(GreenChannelUniform, greenChannelActive);
		normalmapShader.applyShaderBool(BlueChannelUniform, blueChannelActive);
		normalmapPanel.draw();

		static char saveLocation[500] = "D:\\scr.tga";
		static char imageLoadLocation[500] = "Resources\\goli.png";
		static std::string path;
		if (shouldSaveNormalMap)
		{
			SaveNormalMapToFile(saveLocation);
			glfwSetWindowSize(window, prevWindowSize.x, prevWindowSize.y);
			shouldSaveNormalMap = false;
			continue;
		}

		if (isKeyPressed(GLFW_KEY_F10))
		{
			shouldSaveNormalMap = true;
			changeSize = true;
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDisable(GL_DEPTH_TEST);
		glClearColor(0.1f, 0.1f, 0.1f, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		frameShader.use();
		frameShader.applyShaderUniformMatrix(frameModelMatrixUniform, frameDrawingPanel.getTransform()->getMatrix());
		frameDrawingPanel.setTextureID(fbs.getBufferTexture());
		frameDrawingPanel.draw();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		previewFbs.BindFrameBuffer();
		glEnable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		static float rot = 0;
		static glm::vec3 diffuseColour = glm::vec3(1, 1, 1);
		static glm::vec3 ambientColour = glm::vec3(0.14f, 0.14f, 0.14f);
		static glm::vec3 lightColour = glm::vec3(1, 1, 1);
		rot += 0.1f;
		modelViewShader.use();
		modelViewShader.applyShaderUniformMatrix(modelViewmodelUniform, glm::rotate(glm::mat4(), glm::radians(rot += (modelPreviewRotationSpeed - 0.1f)), glm::vec3(glm::sin(rot * 0.1f), 0.2f, 1.0f)));
		modelViewShader.applyShaderUniformMatrix(modelVieviewUniform, glm::translate(glm::mat4(), glm::vec3(0.0f, 0.0f, modelPreviewZoomLevel)));
		modelViewShader.applyShaderUniformMatrix(modelViewprojectionUniform, glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f));
		modelViewShader.applyShaderInt(modelNormalMapModeUniform, modelViewMode);
		modelViewShader.applyShaderFloat(modelNormalMapStrengthUniform, normalMapStrength);
		modelViewShader.applyShaderFloat(modelLightIntensityUniform, lightIntensity);
		modelViewShader.applyShaderFloat(modelLightSpecularityUniform, specularity);
		modelViewShader.applyShaderFloat(modelLightSpecularityStrengthUniform, specularityStrength);
		modelViewShader.applyShaderVector3(modelLightDirectionUniform, glm::normalize(lightDirection));
		modelViewShader.applyShaderInt(modelHeightMapTextureUniform, 0);
		modelViewShader.applyShaderInt(modelTextureMapTextureUniform, 1);

		modelViewShader.applyShaderVector3(modelDiffuseColourUniform, diffuseColour);
		modelViewShader.applyShaderVector3(modelLightColourUniform, lightColour);
		modelViewShader.applyShaderVector3(modelAmbientColourUniform, ambientColour);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, heightmapTexId);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, penguinTextureId);
		modelPreviewObj->draw();
		glActiveTexture(GL_TEXTURE0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		if (windowWidth < windowHeight)
		{
			float scale = 1.0f;
			brushPanel.getTransform()->setScale(glm::vec2((brushData.brushScale / texData.getWidth())*scale, (brushData.brushScale / texData.getHeight())*aspectRatio) * zoomLevel * 2.0f);
		}
		else
		{
			float scale = (float)1;
			if (windowHeight < texData.getHeight())
				scale = 1;
			brushPanel.getTransform()->setScale(glm::vec2((brushData.brushScale / texData.getWidth()) / aspectRatio, (brushData.brushScale / texData.getHeight())*scale) * zoomLevel * 2.0f);
		}
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		brushPreviewShader.use();
		brushPanel.getTransform()->setPosition(((x / windowWidth)*2.0f) - 1.0f, -(((y / windowHeight)*2.0f) - 1.0f));
		brushPanel.getTransform()->update();
		brushPreviewShader.applyShaderFloat(brushPreviewStrengthUniform, brushData.brushStrength);
		brushPreviewShader.applyShaderFloat(brushPreviewOffsetUniform, brushData.brushOffset);
		brushPreviewShader.applyShaderVector3(brushPreviewColourUniform, (brushData.heightMapPositiveDir ? glm::vec3(1) : glm::vec3(0)));
		brushPreviewShader.applyShaderUniformMatrix(brushPreviewModelUniform, brushPanel.getTransform()->getMatrix());
		brushPanel.draw();

		ImGui_ImplOpenGL2_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		WindowTopBarDisplay(minimizeTextureId, restoreTextureId, isMaximized, closeTextureId);

		ImGuiWindowFlags window_flags = 0;
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 1.0f);
		window_flags = 0;
		window_flags |= ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoResize;
		window_flags |= ImGuiWindowFlags_NoCollapse;
		window_flags |= ImGuiWindowFlags_NoTitleBar;
		bool *p_open = NULL;

		BottomBarDisplay(p_open, window_flags);
		SideBarDisplay(p_open, window_flags, isFullscreen, mode, frameDrawingPanel, imageLoadLocation,
			fileExplorer, path, updateImageLocation, saveLocation, shouldSaveNormalMap, changeSize, mapDrawViewMode);
		DisplayBrushSettingsUserInterface(isBlurOn, brushData);

		if (mapDrawViewMode < 3)
		{
			DisplayNormalSettingsUserInterface(normalMapStrength, flipX_Ydir, redChannelActive, greenChannelActive, blueChannelActive);
			if (mapDrawViewMode == 2)
				DisplayLightSettingsUserInterface(lightIntensity, specularity, specularityStrength, lightDirection);
		}

		ImGui::PopStyleColor();
		ImGui::PopStyleVar();
		ImGui::PopStyleVar();
		ImGui::PopItemWidth();
		ImGui::End();
		ImGui::PopStyleVar();
		ImGui::PopStyleVar();
		ImGui::PopStyleVar();

		//________Preview Display_______
		DisplayPreview(p_open, window_flags, modelViewMode, diffuseColour, ambientColour, lightColour);
		fileExplorer.display();
		ImGui::Render();

		glBindVertexArray(0);
		glUseProgram(0);
		ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

		if (path != prevPath)
		{
			TextureManager::getTextureDataFromFile(path, texData);
			heightmapTexId = TextureManager::loadTextureFromData(texData, false);
			normalmapPanel.setTextureID(heightmapTexId);
			for (int i = 0; i < path.length(); i++)
				imageLoadLocation[i] = path[i];
			updateImageLocation = false;
			prevPath = path;
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	ImGui_ImplOpenGL2_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}
inline void SideBarDisplay(bool * p_open, const ImGuiWindowFlags &window_flags, bool &isFullscreen, const GLFWvidmode * mode, DrawingPanel &frameDrawingPanel, char  imageLoadLocation[500], FileExplorer &fileExplorer, std::string &path, bool &updateImageLocation, char  saveLocation[500], bool &shouldSaveNormalMap, bool &changeSize, int &mapDrawViewMode)
{
	ImGui::SetNextWindowPos(ImVec2(windowWidth - 5, 42), ImGuiSetCond_Always);
	ImGui::SetNextWindowSize(ImVec2(10, windowHeight - 67), ImGuiSetCond_Always);
	ImGui::Begin("Side_Bar", p_open, window_flags);
	ImGui::End();

	ImGui::SetNextWindowPos(ImVec2(0, 42), ImGuiSetCond_Always);
	ImGui::SetNextWindowSize(ImVec2(glm::clamp(windowWidth * 0.15f, 280.0f, 600.0f), windowHeight - 67), ImGuiSetCond_Always);
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
	if (ImGui::Button("Reset View", ImVec2(ImGui::GetContentRegionAvailWidth(), 40)))
	{
		frameDrawingPanel.getTransform()->setPosition(0, 0);
		zoomLevel = 1;
	}
	ImGui::Spacing();
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 5));
	ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth() / 1.45f);
	ImGui::InputText("## Load location", imageLoadLocation, sizeof(imageLoadLocation));
	ImGui::PopItemWidth();
	ImGui::SameLine(0, 5);
	if (ImGui::Button("LOAD", ImVec2(ImGui::GetContentRegionAvailWidth(), 27)))
	{
		fileExplorer.displayDialog(&path, FileType::IMAGE);
		updateImageLocation = true;
	}
	ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth() / 1.45f);
	ImGui::InputText("## Save location", saveLocation, sizeof(saveLocation));
	ImGui::PopItemWidth();
	ImGui::SameLine(0, 5);
	if (ImGui::Button("EXPORT", ImVec2(ImGui::GetContentRegionAvailWidth(), 27))) { shouldSaveNormalMap = true; changeSize = true; }
	ImGui::PopStyleVar();
	ImGui::Spacing();
	ImGui::Text("VIEW MODE");
	ImGui::Separator();
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 10));
	int modeButtonWidth = (int)(ImGui::GetContentRegionAvailWidth() / 3.0f);
	ImGui::Spacing();

	if (mapDrawViewMode == 3) ImGui::PushStyleColor(ImGuiCol_Button, ACCENT_COL);
	else ImGui::PushStyleColor(ImGuiCol_Button, SECONDARY_COL);
	if (ImGui::Button("Height", ImVec2(modeButtonWidth - 5, 40))) { mapDrawViewMode = 3; }
	ImGui::PopStyleColor();

	ImGui::SameLine(0, 5);
	if (mapDrawViewMode == 1) ImGui::PushStyleColor(ImGuiCol_Button, ACCENT_COL);
	else ImGui::PushStyleColor(ImGuiCol_Button, SECONDARY_COL);
	if (ImGui::Button("Normal", ImVec2(modeButtonWidth - 5, 40))) { mapDrawViewMode = 1; }
	ImGui::PopStyleColor();

	ImGui::SameLine(0, 5);
	if (mapDrawViewMode == 2) ImGui::PushStyleColor(ImGuiCol_Button, ACCENT_COL);
	else ImGui::PushStyleColor(ImGuiCol_Button, SECONDARY_COL);
	if (ImGui::Button("3D Plane", ImVec2(modeButtonWidth, 40))) { mapDrawViewMode = 2; }
	ImGui::PopStyleColor();
	ImGui::PopStyleVar();
}
void BottomBarDisplay(bool * p_open, const ImGuiWindowFlags &window_flags)
{
	ImGui::SetNextWindowPos(ImVec2(0, windowHeight - 25), ImGuiSetCond_Always);
	ImGui::SetNextWindowSize(ImVec2(windowWidth, 25), ImGuiSetCond_Always);
	ImGui::Begin("Bottom_Bar", p_open, window_flags);
	ImGui::Indent(ImGui::GetContentRegionAvailWidth()*0.5f - 30);
	ImGui::Text("v0.8 - Alpha");
	ImGui::End();
}
void SetupImGui()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL2_Init();
	// Setup ImGui Theme
	CustomColourImGuiTheme();
	ImFont* font = io.Fonts->AddFontFromFileTTF("Resources\\arial.ttf", 16.0f);
	IM_ASSERT(font != NULL);
}
void SetStatesForSavingNormalMap(bool &changeSize, glm::vec2 &prevWindowSize, int &retflag)
{
	retflag = 1;
	glViewport(0, 0, texData.getWidth(), texData.getHeight());
	if (changeSize)
	{
		prevWindowSize = glm::vec2(windowWidth, windowHeight);
		glfwSetWindowSize(window, texData.getWidth() + 100, texData.getHeight() + 100);
		changeSize = false;
		{ retflag = 3; return; };
	}
}
void HandleKeyboardInput(float &normalMapStrength, double deltaTime, int &mapDrawViewMode, DrawingPanel &frameDrawingPanel, bool &isMaximized)
{
	if (isKeyPressed(GLFW_KEY_A))
		normalMapStrength += 2.5f * deltaTime;
	if (isKeyPressed(GLFW_KEY_D))
		normalMapStrength -= 2.5f * deltaTime;
	if (isKeyPressed(GLFW_KEY_W))
		zoomLevel += zoomLevel * 1.5f * deltaTime;
	if (isKeyPressed(GLFW_KEY_S))
		zoomLevel -= zoomLevel * 1.5f * deltaTime;
	zoomLevel = glm::clamp(zoomLevel, 0.1f, 5.0f);

	if (isKeyPressed(GLFW_KEY_J))
		mapDrawViewMode = 1;
	if (isKeyPressed(GLFW_KEY_K))
		mapDrawViewMode = 2;
	if (isKeyPressed(GLFW_KEY_L))
		mapDrawViewMode = 3;

	if (isKeyPressed(GLFW_KEY_LEFT))
		frameDrawingPanel.getTransform()->translate(-1.0f * deltaTime, 0.0f);
	if (isKeyPressed(GLFW_KEY_RIGHT))
		frameDrawingPanel.getTransform()->translate(1.0f * deltaTime, 0);
	if (isKeyPressed(GLFW_KEY_UP))
		frameDrawingPanel.getTransform()->translate(0.0f, 1.0f * deltaTime);
	if (isKeyPressed(GLFW_KEY_DOWN))
		frameDrawingPanel.getTransform()->translate(0.0f, -1.0f * deltaTime);
	if (isKeyPressed(GLFW_KEY_8))
		frameDrawingPanel.getTransform()->rotate(1.0f * deltaTime);

	if (isKeyPressed(GLFW_KEY_V))
	{
		glfwSetWindowSize(window, 1600, 800);
		isMaximized = false;
	}
}
inline void DisplayBrushSettingsUserInterface(bool &isBlurOn, BrushData &brushData)
{
	ImGui::Text("BRUSH SETTINGS");
	ImGui::Separator();
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 10));
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 5));
	ImGui::Spacing();
	ImGui::PushStyleColor(ImGuiCol_SliderGrab, SECONDARY_COL);

	if (ImGui::Button((isBlurOn) ? "HEIGHT MODE" : "BLUR MODE", ImVec2((int)(ImGui::GetContentRegionAvailWidth() / 2.0f), 40)))
		isBlurOn = !isBlurOn;
	ImGui::SameLine();

	if (isBlurOn)
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);

	if (ImGui::Button((brushData.heightMapPositiveDir) ? "Height -VE" : "Height +VE", ImVec2(ImGui::GetContentRegionAvailWidth(), 40)))
		brushData.heightMapPositiveDir = (isBlurOn) ? brushData.heightMapPositiveDir : !brushData.heightMapPositiveDir;
	if (isBlurOn)
		ImGui::PopStyleVar();

	if (ImGui::SliderFloat(" Brush Scale", &brushData.brushScale, 1.0f, texData.getHeight()*0.5f, "%.2f", 1.0f)) {}
	if (ImGui::SliderFloat(" Brush Offset", &brushData.brushOffset, 0.01f, 10.0f, "%.2f", 1.0f)) {}
	if (ImGui::SliderFloat(" Brush Strength", &brushData.brushStrength, 0.0f, 1.0f, "%0.2f", 1.0f)) {}
	if (ImGui::SliderFloat(" Brush Min Height", &brushData.brushMinHeight, 0.0f, 1.0f, "%0.2f", 1.0f)) {}
	if (ImGui::SliderFloat(" Brush Max Height", &brushData.brushMaxHeight, 0.0f, 1.0f, "%0.2f", 1.0f)) {}
	if (ImGui::SliderFloat(" Brush Draw Rate", &brushData.brushRate, 0.0f, texData.getHeight() / 2, "%0.2f", 1.0f)) {}
	static BrushData bCopy = BrushData();
	if (bCopy != brushData)
		bCopy = brushData;

	if (brushData.brushMinHeight > brushData.brushMaxHeight)
		brushData.brushMinHeight = brushData.brushMaxHeight;
	else if (brushData.brushMaxHeight < brushData.brushMinHeight)
		brushData.brushMaxHeight = brushData.brushMinHeight;
}
inline void DisplayNormalSettingsUserInterface(float &normalMapStrength, bool &flipX_Ydir, bool &redChannelActive, bool &greenChannelActive, bool &blueChannelActive)
{
	ImGui::Text("NORMAL SETTINGS");
	ImGui::Separator();
	if (ImGui::SliderFloat(" Normal Strength", &normalMapStrength, -100.0f, 100.0f, "%.2f")) {}
	if (ImGui::Button("Flip X-Y", ImVec2(ImGui::GetContentRegionAvailWidth(), 40))) { flipX_Ydir = !flipX_Ydir; }
	float width = ImGui::GetContentRegionAvailWidth() / 3.0f - 7;

	if (!redChannelActive) ImGui::PushStyleColor(ImGuiCol_Button, ACCENT_COL);
	else ImGui::PushStyleColor(ImGuiCol_Button, SECONDARY_COL);
	if (ImGui::Button("R", ImVec2(width, 40))) { redChannelActive = !redChannelActive; } ImGui::SameLine();
	ImGui::PopStyleColor();

	if (!greenChannelActive) ImGui::PushStyleColor(ImGuiCol_Button, ACCENT_COL);
	else ImGui::PushStyleColor(ImGuiCol_Button, SECONDARY_COL);
	if (ImGui::Button("G", ImVec2(width, 40))) { greenChannelActive = !greenChannelActive; } ImGui::SameLine();
	ImGui::PopStyleColor();

	if (!blueChannelActive) ImGui::PushStyleColor(ImGuiCol_Button, ACCENT_COL);
	else ImGui::PushStyleColor(ImGuiCol_Button, SECONDARY_COL);
	if (ImGui::Button("B", ImVec2(width, 40))) { blueChannelActive = !blueChannelActive; }
	ImGui::PopStyleColor();
}
inline void DisplayLightSettingsUserInterface(float &lightIntensity, float &specularity, float &specularityStrength, glm::vec3 &lightDirection)
{
	ImGui::Text("LIGHT SETTINGS");
	ImGui::Separator();
	if (ImGui::SliderFloat(" Diffuse Intensity", &lightIntensity, 0.0f, 1.0f, "%.2f")) {}
	if (ImGui::SliderFloat(" Specularity", &specularity, 0.01f, 10.0f, "%.2f")) {}
	if (ImGui::SliderFloat(" Specularity Strength", &specularityStrength, 0.0f, 1.0f, "%.2f")) {}
	ImGui::Text("Light Direction");
	ImGui::PushItemWidth((ImGui::GetContentRegionAvailWidth() / 3.0f) - 7);
	if (ImGui::SliderFloat("## X Angle", &lightDirection.x, 0.01f, 179.99f, "X:%.2f")) {}
	ImGui::SameLine();
	if (ImGui::SliderFloat("## Y Angle", &lightDirection.y, 0.01f, 179.99f, "Y:%.2f")) {}
	ImGui::SameLine();
	if (ImGui::SliderFloat("## Z Angle", &lightDirection.z, 0.01f, 179.99f, "Z:%.2f")) {}
	ImGui::PopItemWidth();
}
inline void DisplayPreview(bool * p_open, const ImGuiWindowFlags &window_flags, int &modelViewMode, glm::vec3 &diffuseColour, glm::vec3 &ambientColour, glm::vec3 &lightColour)
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 1.0f);
	ImGui::PushStyleColor(ImGuiCol_SliderGrab, SECONDARY_COL);
	ImGui::SetNextWindowPos(ImVec2(windowWidth - 305, 42), ImGuiSetCond_Always);
	ImGui::SetNextWindowSize(ImVec2(300, windowHeight - 67), ImGuiSetCond_Always);
	ImGui::Begin("Preview_Bar", p_open, window_flags);
	ImGui::Image((ImTextureID)previewFbs.getBufferTexture(), ImVec2(300, 300));

	const char* items[] = { "CUBE", "CYLINDER", "SPHERE", "TORUS" };
	static const char* current_item = items[0];
	ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth() + 5);

	if (ImGui::BeginCombo("##combo", current_item)) // The second parameter is the label previewed before opening the combo.
	{
		for (int n = 0; n < IM_ARRAYSIZE(items); n++)
		{
			bool is_selected = (current_item == items[n]); // You can store your selection however you want, outside or inside your objects
			if (ImGui::Selectable(items[n], is_selected))
			{
				current_item = items[n];
				delete modelPreviewObj;
				modelPreviewObj = nullptr;
				switch (n)
				{
				case 0:
					modelPreviewObj = modelLoader.CreateModelFromFile(CUBE_MODEL_PATH);
					break;
				case 1:
					modelPreviewObj = modelLoader.CreateModelFromFile(CYLINDER_MODEL_PATH);
					break;
				case 2:
					modelPreviewObj = modelLoader.CreateModelFromFile(SPHERE_MODEL_PATH);
					break;
				case 3:
					modelPreviewObj = modelLoader.CreateModelFromFile(TORUS_MODEL_PATH);
					break;
				default:
					break;
				}
			}
			if (is_selected)
				ImGui::SetItemDefaultFocus();   // You may set the initial focus when opening the combo (scrolling + for keyboard navigation support)
		}
		ImGui::EndCombo();
	}

	ImGui::SliderFloat("##Rotation speed", &modelPreviewRotationSpeed, 0, 1, "Rotation Speed:%.2f");
	ImGui::SliderFloat("##Zoom level", &modelPreviewZoomLevel, -1.0f, -100.0f, "Zoom Level:%.2f");
	ImGui::PopItemWidth();
	ImGui::Spacing();
	ImGui::Text("VIEW MODE");
	ImGui::Separator();
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 10));
	int modeButtonWidth = (int)(ImGui::GetContentRegionAvailWidth() / 4.0f);
	ImGui::Spacing();

	if (modelViewMode == 3) ImGui::PushStyleColor(ImGuiCol_Button, ACCENT_COL);
	else ImGui::PushStyleColor(ImGuiCol_Button, SECONDARY_COL);
	if (ImGui::Button("Height", ImVec2(modeButtonWidth - 5, 40))) { modelViewMode = 3; }
	ImGui::PopStyleColor();

	ImGui::SameLine(0, 5);
	if (modelViewMode == 1) ImGui::PushStyleColor(ImGuiCol_Button, ACCENT_COL);
	else ImGui::PushStyleColor(ImGuiCol_Button, SECONDARY_COL);
	if (ImGui::Button("Normal", ImVec2(modeButtonWidth - 5, 40))) { modelViewMode = 1; }
	ImGui::PopStyleColor();

	ImGui::SameLine(0, 5);
	if (modelViewMode == 2) ImGui::PushStyleColor(ImGuiCol_Button, ACCENT_COL);
	else ImGui::PushStyleColor(ImGuiCol_Button, SECONDARY_COL);
	if (ImGui::Button("3D Plane", ImVec2(modeButtonWidth, 40))) { modelViewMode = 2; }
	ImGui::PopStyleColor();

	ImGui::SameLine(0, 5);
	if (modelViewMode == 4) ImGui::PushStyleColor(ImGuiCol_Button, ACCENT_COL);
	else ImGui::PushStyleColor(ImGuiCol_Button, SECONDARY_COL);
	if (ImGui::Button("Textured", ImVec2(modeButtonWidth, 40))) { modelViewMode = 4; }
	ImGui::PopStyleColor();

	ImGui::PopStyleVar();

	if (modelViewMode == 2 || modelViewMode == 4)
	{
		ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth() + 5);
		ImGui::Text("Diffuse Colour");
		ImGui::ColorEdit3("Diffuse Color", &diffuseColour[0]);
		ImGui::Text("Ambient Colour");
		ImGui::ColorEdit3("Ambient Color", &ambientColour[0]);
		ImGui::Text("Light Colour");
		ImGui::ColorEdit3("Light Color", &lightColour[0]);
		ImGui::PopItemWidth();
	}
	ImGui::End();

	ImGui::PopStyleColor();
	ImGui::PopStyleVar(3);
}
inline void WindowTopBarDisplay(unsigned int minimizeTexture, unsigned int restoreTexture, bool &isMaximized, unsigned int closeTexture)
{
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 13));
	if (ImGui::BeginMainMenuBar())
	{
		ImGui::Indent(20);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(25, 5));
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open Project", "CTRL+O"))
			{
			}
			if (ImGui::MenuItem("Open Scene")) {}
			ImGui::EndMenu();
		}

		const char* items[] = { "    Default Theme", "    Dark Theme", "    Light Theme", "    Blue Theme" };
		static int item_current = 0;
		ImGui::PushItemWidth(180);
		ImGui::Combo("##combo", &item_current, items, IM_ARRAYSIZE(items));
		ImGui::PopItemWidth();
		switch (item_current)
		{
		case 0:
			CustomColourImGuiTheme();
			break;
		case 1:
			ImGui::StyleColorsDark();
			isUsingCustomTheme = false;
			break;
		case 2:
			ImGui::StyleColorsLight();
			isUsingCustomTheme = false;
			break;
		case 3:
			ImGui::StyleColorsClassic();
			isUsingCustomTheme = false;
			break;
		default:
			break;
		}
		ImGui::PopStyleVar();
		ImGui::Indent(windowWidth - 160);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(5, 10));
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		if (isUsingCustomTheme)
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ACCENT_COL);
		if (ImGui::ImageButton((ImTextureID)minimizeTexture, ImVec2(30, 30), ImVec2(0, 0), ImVec2(1, 1), 5)) { glfwIconifyWindow(window); }
		if (ImGui::ImageButton((ImTextureID)restoreTexture, ImVec2(30, 30), ImVec2(0, 0), ImVec2(1, 1), 5))
		{
			if (!isMaximized)
			{
				glfwMaximizeWindow(window);
				isMaximized = true;
			}
			else
			{
				glfwSetWindowSize(window, 800, 800);
				isMaximized = false;
			}
		}
		if (ImGui::ImageButton((ImTextureID)closeTexture, ImVec2(30, 30), ImVec2(0, 0), ImVec2(1, 1), 5)) { glfwSetWindowShouldClose(window, true); }
		ImGui::PopStyleColor();
		if (isUsingCustomTheme)
			ImGui::PopStyleColor();
		ImGui::PopStyleVar();
	}
	ImGui::EndMainMenuBar();
	ImGui::PopStyleVar();
}
void SaveNormalMapToFile(char  saveLocation[500])
{
	std::string locationStr = std::string(saveLocation);
	if (locationStr.length() > 4)
	{
		if (exportImage(locationStr, 0, 0, texData.getWidth(), texData.getHeight()))
			std::cout << "Saved at " << locationStr;
	}
}
inline void HandleLeftMouseButtonInput_NormalMapInteraction(int state, glm::vec2 &prevMouseCoord, BrushData &brushData, DrawingPanel &frameBufferPanel, bool isBlurOn)
{
	if (state == GLFW_PRESS)
	{
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		glm::vec2 currentMouseCoord(xpos, ypos);

		glm::vec2 windowNormalizedCurrentMouseCoord = glm::vec2(currentMouseCoord.x / windowWidth, currentMouseCoord.y / windowHeight);
		glm::vec2 windowNormalizedPrevMouseCoord = glm::vec2(prevMouseCoord.x / windowWidth, prevMouseCoord.y / windowHeight);
		float minDistThresholdForDraw = (brushData.brushRate / texData.getWidth()) * zoomLevel;
		float distOfPrevAndCurrentMouseCoord = glm::distance(windowNormalizedCurrentMouseCoord, windowNormalizedPrevMouseCoord);

		if (currentMouseCoord != prevMouseCoord && distOfPrevAndCurrentMouseCoord > minDistThresholdForDraw)
		{
			xpos = xpos / windowWidth; // window size normalized width
			ypos = 1.0f - (ypos / windowHeight); // window size normalized height

			std::cout << "\nCursor pos : " << xpos << ", " << ypos;
			/*glm::vec4 wD = frameBufferPanel.getPanelWorldDimension();
			std::cout << "\nLeft side loc bound : " << wD.x;*/
			float leftBound = (frameBufferPanel.getTransform()->getPosition().x + 1) * 0.5f - 0.25f;
			float rightBound = (frameBufferPanel.getTransform()->getPosition().x + 1) * 0.5f + 0.25f;
			float topBound = (frameBufferPanel.getTransform()->getPosition().y + 1) * 0.5f + 0.25f;
			float bottomBound = (frameBufferPanel.getTransform()->getPosition().y + 1) * 0.5f - 0.25f;
			std::cout << "\nLeft bound = " << leftBound << " Right bound = " << rightBound
				<< " Top bound = " << topBound << " Bottom bound = " << bottomBound;

			if (xpos >= leftBound && xpos <= rightBound /*&& ypos >= bottomBound && ypos <= topBound*//*frameBufferPanel.isPointInPanel(xpos, ypos)*/) //not working correctly
			{
				std::cout << "\nIs inside";
				float prevX = prevMouseCoord.x / windowWidth;
				float prevY = 1.0f - (prevMouseCoord.y / windowHeight);

				glm::vec4 worldDimensions = frameBufferPanel.getPanelWorldDimension();

				xpos = (xpos - leftBound) / (rightBound - leftBound);//((xpos - worldDimensions.x) / (worldDimensions.y - worldDimensions.x)) + (frameBufferPanel.getTransform()->getPosition().x * zoomLevel * 0.5f); //works at default zoom as 0.5
				ypos = ((ypos - worldDimensions.w) / (worldDimensions.z - worldDimensions.w)) + (frameBufferPanel.getTransform()->getPosition().y * zoomLevel * 0.5f);

				const float maxWidth = texData.getWidth();
				const float convertedBrushScale = (brushData.brushScale / texData.getHeight()) * maxWidth * 2.5f;

				float left = (xpos - convertedBrushScale) * maxWidth;
				float right = (xpos + convertedBrushScale) * maxWidth;
				float bottom = (ypos - convertedBrushScale) * maxWidth;
				float top = (ypos + convertedBrushScale) * maxWidth;

				if (!isBlurOn)
				{
					if (distOfPrevAndCurrentMouseCoord > 0.02f)
					{
						prevX = (prevX - leftBound) / (rightBound - leftBound);//((prevX - worldDimensions.x) / (worldDimensions.y - worldDimensions.x)) + (frameBufferPanel.getTransform()->getPosition().x * zoomLevel * 0.5f); //works at default zoom as 0.5
						prevY = ((prevY - worldDimensions.w) / (worldDimensions.z - worldDimensions.w)) + (frameBufferPanel.getTransform()->getPosition().y * zoomLevel * 0.5f);
						glm::vec2 prevPoint(prevX, prevY);
						glm::vec2 toPoint(xpos, ypos);
						glm::vec2 curPoint = prevPoint;

						distOfPrevAndCurrentMouseCoord = glm::min(distOfPrevAndCurrentMouseCoord, 0.05f);

						int numberOfPoints = (distOfPrevAndCurrentMouseCoord / 0.05f) * 5;
						glm::vec2 incValue = (prevPoint - toPoint) * (1.0f / numberOfPoints);

						for (int i = 0; i < numberOfPoints; i++)
						{
							left = curPoint.x * maxWidth - convertedBrushScale;
							right = curPoint.x * maxWidth + convertedBrushScale;
							bottom = curPoint.y * maxWidth - convertedBrushScale;
							top = curPoint.y * maxWidth + convertedBrushScale;

							left = glm::clamp(left, 0.0f, maxWidth);
							right = glm::clamp(right, 0.0f, maxWidth);
							bottom = glm::clamp(bottom, 0.0f, maxWidth);
							top = glm::clamp(top, 0.0f, maxWidth);

							curPoint += incValue;
							SetPixelValues(texData, left, right, bottom, top, curPoint.x, curPoint.y, brushData);
						}
					}
					else
					{
						left = glm::clamp(left, 0.0f, maxWidth);
						right = glm::clamp(right, 0.0f, maxWidth);
						bottom = glm::clamp(bottom, 0.0f, maxWidth);
						top = glm::clamp(top, 0.0f, maxWidth);
						SetPixelValues(texData, left, right, bottom, top, xpos, ypos, brushData);
					}
				}
				else if (isBlurOn)
				{
					left = glm::clamp(left, 0.0f, maxWidth);
					right = glm::clamp(right, 0.0f, maxWidth);
					bottom = glm::clamp(bottom, 0.0f, maxWidth);
					top = glm::clamp(top, 0.0f, maxWidth);
					SetBluredPixelValues(texData, left, right, bottom, top, xpos, ypos, brushData);
				}

				GLenum format = TextureManager::getTextureFormatFromData(4);
				glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texData.getWidth(), texData.getHeight(), format, GL_UNSIGNED_BYTE, texData.getTextureData());
				prevMouseCoord = currentMouseCoord;
			}
		}
	}
	else
	{
		prevMouseCoord = glm::vec2(-10, -10);
	}
}

inline void HandleLeftMouseButtonInput_UI(int state, glm::vec2 &initPos, WindowSide &windowSideAtInitPos, double x, double y, bool &isMaximized, glm::vec2 &prevGlobalFirstMouseCoord)
{
	if (state == GLFW_PRESS)
	{
		if (initPos == glm::vec2(-1000, -1000))
		{
			windowSideAtInitPos = WindowTransformUtility::GetWindowSideAtMouseCoord((int)x, (int)y, windowWidth, windowHeight);
			initPos = glm::vec2(x, y);
		}

		if (y < 40 && y >= WindowTransformUtility::BORDER_SIZE)
		{
			if (windowSideAtInitPos == WindowSide::NONE)
			{
				glm::vec2 currentPos(x, y);
				if (prevGlobalFirstMouseCoord != currentPos && prevGlobalFirstMouseCoord != glm::vec2(-500, -500))
				{
					int winPosX, winPosY;
					glfwGetWindowPos(window, &winPosX, &winPosY);
					glfwSetWindowPos(window, winPosX + currentPos.x - initPos.x, winPosY + currentPos.y - initPos.y);
				}
			}
		}
		prevGlobalFirstMouseCoord = glm::vec2(x, y);
	}
	else
	{
		if (windowSideAtInitPos == WindowSide::LEFT)
		{
			glm::vec2 currentPos(x, y);
			if (initPos != currentPos && initPos != glm::vec2(-1000, -1000))
			{
				int xPos, yPos;
				glfwGetWindowPos(window, &xPos, &yPos);
				glfwSetWindowPos(window, xPos + currentPos.x, yPos);
				const glm::vec2 diff = (currentPos + glm::vec2(xPos, 0)) - (initPos + glm::vec2(xPos, 0));
				glfwSetWindowSize(window, windowWidth - diff.x, windowHeight);
			}
		}
		else if (windowSideAtInitPos == WindowSide::RIGHT)
		{
			glm::vec2 currentPos(x, y);
			if (initPos != currentPos && initPos != glm::vec2(-1000, -1000))
			{
				glm::vec2 diff = currentPos - initPos;
				glfwSetWindowSize(window, windowWidth + diff.x, windowHeight);
			}
		}
		else if (windowSideAtInitPos == WindowSide::BOTTOM_RIGHT)
		{
			glm::vec2 currentPos(x, y);
			if (initPos != currentPos && initPos != glm::vec2(-1000, -1000))
			{
				glm::vec2 diff = currentPos - initPos;
				glfwSetWindowSize(window, windowWidth + diff.x, windowHeight + diff.y);
			}
		}
		else if (windowSideAtInitPos == WindowSide::TOP)
		{
			glm::vec2 currentPos(x, y);
			if (initPos != currentPos && initPos != glm::vec2(-1000, -1000))
			{
				glm::vec2 diff = currentPos - initPos;
				glfwSetWindowSize(window, windowWidth, windowHeight - diff.y);
				windowHeight -= diff.y;
				int xPos, yPos;
				glfwGetWindowPos(window, &xPos, &yPos);
				glfwSetWindowPos(window, xPos + currentPos.x - initPos.x, yPos + currentPos.y - initPos.y);
			}
		}
		else if (windowSideAtInitPos == WindowSide::BOTTOM)
		{
			glm::vec2 currentPos(x, y);
			if (initPos != currentPos && initPos != glm::vec2(-1000, -1000))
			{
				const glm::vec2 diff = currentPos - initPos;
				glfwSetWindowSize(window, windowWidth, windowHeight + diff.y);
			}
		}
		else if (windowSideAtInitPos == WindowSide::BOTTOM_LEFT)
		{
			glm::vec2 currentPos(x, y);
			if (initPos != currentPos && initPos != glm::vec2(-1000, -1000))
			{
				glm::vec2 diff = currentPos - initPos;
				glfwSetWindowSize(window, windowWidth - diff.x, windowHeight + diff.y);
				int xPos, yPos;
				glfwGetWindowPos(window, &xPos, &yPos);
				glfwSetWindowPos(window, xPos + currentPos.x, yPos);
			}
		}
		windowSideAtInitPos = WindowSide::NONE;
		initPos = glm::vec2(-1000, -1000);
		prevGlobalFirstMouseCoord = glm::vec2(-500, -500);
	}
}

inline void HandleMiddleMouseButtonInput(int state, glm::vec2 &prevMiddleMouseButtonCoord, double deltaTime, DrawingPanel &frameBufferPanel)
{
	if (state == GLFW_PRESS)
	{
		double x, y;
		glfwGetCursorPos(window, &x, &y);
		glm::vec2 currentPos(x, y);
		glm::vec2 diff = (currentPos - prevMiddleMouseButtonCoord) * glm::vec2(1.0f / windowWidth, 1.0f / windowHeight) * 2.0f;
		frameBufferPanel.getTransform()->translate(diff.x, -diff.y);
		prevMiddleMouseButtonCoord = currentPos;
	}
	else
	{
		double x, y;
		glfwGetCursorPos(window, &x, &y);
		prevMiddleMouseButtonCoord = glm::vec2(x, y);
	}
}

inline void SetPixelValues(TextureData& inputTexData, int startX, int endX, int startY, int endY, double xpos, double ypos, const BrushData& brushData)
{
	ColourData colData;
	float rVal;
	float distance;
	const glm::vec2 pixelPos(xpos, ypos);
	const float px_width = inputTexData.getWidth();
	const float px_height = inputTexData.getHeight();
	const float distanceRemap = brushData.brushScale / px_height;
	for (int i = startX; i < endX; i++)
	{
		for (int j = startY; j < endY; j++)
		{
			colData = inputTexData.getTexelColor(i, j);
			rVal = colData.getColourIn_0_1_Range().r;
			distance = glm::distance(pixelPos, glm::vec2((double)i / px_width, (double)j / px_height));
			if (distance < distanceRemap)
			{
				distance = (1.0f - (distance / distanceRemap)) * brushData.brushOffset;
				distance = glm::clamp(distance, 0.0f, 1.0f) * brushData.brushStrength;
				rVal = rVal + distance * ((brushData.heightMapPositiveDir ? brushData.brushMaxHeight : brushData.brushMinHeight) - rVal);
				ColourData col(rVal, rVal, rVal, 1.0f);
				inputTexData.setTexelColor(col, i, j);
			}
		}
	}
}

inline void SetBluredPixelValues(TextureData& inputTexData, int startX, int endX, int startY, int endY, double xpos, double ypos, const BrushData& brushData)
{
	float rVal;
	float distance;
	glm::vec2 pixelPos(xpos, ypos);
	const float px_width = inputTexData.getWidth();
	const float px_height = inputTexData.getHeight();
	float distanceRemap = brushData.brushScale / px_height;

	//Temp allocation of image section
	const int _width = endX - startX;
	const int _height = endY - startY;
	const int totalPixelCount = _width * _height;
	ColourData *tempPixelData = new ColourData[totalPixelCount];

	for (int i = startX; i < endX; i++)
	{
		for (int j = startY; j < endY; j++)
		{
			int index = (i - startX)*_width + (j - startY);
			if (index >= 0 && index < totalPixelCount)
				tempPixelData[index] = inputTexData.getTexelColor(i, j);
		}
	}

	for (int i = startX; i < endX; i++)
	{
		for (int j = startY; j < endY; j++)
		{
			distance = glm::distance(pixelPos, glm::vec2((double)i / px_width, (double)j / px_height));
			if (distance < distanceRemap - 0.01f)
			{
				int index = (i - startX)*(endX - startX) + (j - startY);
				if (index < 0 || index >= totalPixelCount)
					continue;

				index = (i - startX)*(endX - startX) + (j - startY);
				float avg = tempPixelData[index].getColourIn_0_1_Range().r * 0.5f;

				int leftIndex = ((i - 1) - startX)*(endX - startX) + (j - startY);
				int rightIndex = ((i + 1) - startX)*(endX - startX) + (j - startY);
				int topIndex = (i - startX)*(endX - startX) + ((j + 1) - startY);
				int bottomIndex = (i - startX)*(endX - startX) + ((j - 1) - startY);

				int topLeftIndex = ((i - 1) - startX)*(endX - startX) + ((j + 1) - startY);
				int bottomLeftIndex = ((i - 1) - startX)*(endX - startX) + ((j - 1) - startY);
				int topRightIndex = ((i + 1) - startX)*(endX - startX) + ((j + 1) - startY);
				int bottomRightIndex = ((i + 1) - startX)*(endX - startX) + ((j - 1) - startY);

				int kernel[] = { leftIndex,rightIndex,topIndex,bottomIndex, topLeftIndex,bottomLeftIndex, topRightIndex, bottomRightIndex };
				//not clamping values based in width and heifhgt of current pixel center
				for (unsigned int i = 0; i < 8; i++)
					avg += (kernel[i] >= 0 && kernel[i] < totalPixelCount) ? tempPixelData[kernel[i]].getColourIn_0_1_Range().r * 0.0625f : 0.01f;
				float pixelCol = tempPixelData[index].getColourIn_0_1_Range().r;
				float finalColor = 0.0025f * pixelCol;
				finalColor += avg;
				finalColor = glm::mix(pixelCol, finalColor, brushData.brushStrength);
				finalColor = glm::clamp(finalColor, 0.0f, 1.0f);
				ColourData colData;
				colData.setColour_32_bit(glm::vec4(finalColor, finalColor, finalColor, 1.0f));
				inputTexData.setTexelColor(colData, i, j);
			}
		}
	}
	delete[] tempPixelData;
}

inline void SetBrushPixelValues(TextureData& inputTexData, int startX, int width, int startY, int height, double xpos, double ypos, const BrushData& brushData)
{
	ColourData colData;
	float rVal;
	float distance = 0;
	const glm::vec2 pixelPos(xpos, ypos);
	const float px_width = inputTexData.getWidth();
	const float px_height = inputTexData.getHeight();
	const float distanceRemap = 0.5f;
	for (int i = startX; i < width; i++)
	{
		for (int j = startY; j < height; j++)
		{
			distance = glm::distance(pixelPos, glm::vec2((double)i / px_width, (double)j / px_height));
			ColourData col;
			if (distance < distanceRemap)
			{
				distance = (1.0f - (distance / distanceRemap)) * brushData.brushOffset;
				distance = glm::clamp(distance, 0.0f, 1.0f) * brushData.brushStrength;
				if (brushData.heightMapPositiveDir)
					col.setColour_32_bit(brushData.brushMaxHeight, brushData.brushMaxHeight, brushData.brushMaxHeight, distance);
				else
					col.setColour_32_bit(brushData.brushMinHeight, brushData.brushMinHeight, brushData.brushMinHeight, distance);
			}
			else
				col.setColour_32_bit(0, 0, 0, 0);
			inputTexData.setTexelColor(col, i, j);
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
	width = glm::clamp(width, WINDOW_SIZE_MIN, maxWindowWidth);
	height = glm::clamp(height, WINDOW_SIZE_MIN, maxWindowHeight);

	glfwSetWindowSize(window, width, height);
	windowWidth = width;
	windowHeight = height;
	fbs.updateTextureDimensions(windowWidth, windowHeight);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	zoomLevel += zoomLevel * 0.1f * yoffset;
}

bool exportImage(const std::string& filename, int xOff, int yOff, int w, int h)
{
	//This prevents the images getting padded 
	// when the width multiplied by 3 is not a multiple of 4
	//fbs.BindBufferTexture();
	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	const int nSize = w * h * 3;
	// First let's create our buffer, 3 channels per Pixel
	char* dataBuffer = new char[nSize];

	if (!dataBuffer) return false;
	glReadPixels((GLint)xOff, (GLint)yOff, (GLint)w, (GLint)h, GL_BGR, GL_UNSIGNED_BYTE, dataBuffer);

	//glGetTexImage(GL_TEXTURE_2D, 0, GL_BGR, GL_UNSIGNED_BYTE, dataBuffer);

	//stbi_write_bmp(filename.c_str(), w, h, 3, dataBuffer);

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
	delete[] dataBuffer;
	return true;
}
void CustomColourImGuiTheme(ImGuiStyle* dst)
{
	isUsingCustomTheme = true;
	ImGuiStyle* style = dst ? dst : &ImGui::GetStyle();
	ImVec4* colors = style->Colors;

	colors[ImGuiCol_Text] = WHITE;
	colors[ImGuiCol_TextDisabled] = SECONDARY_COL;
	colors[ImGuiCol_WindowBg] = PRIMARY_COL;
	colors[ImGuiCol_ChildBg] = PRIMARY_COL;
	colors[ImGuiCol_PopupBg] = PRIMARY_COL;
	colors[ImGuiCol_Border] = ACCENT_COL;
	colors[ImGuiCol_BorderShadow] = DARK_GREY;
	colors[ImGuiCol_FrameBg] = ACCENT_COL;
	colors[ImGuiCol_FrameBgHovered] = PRIMARY_COL;
	colors[ImGuiCol_FrameBgActive] = SECONDARY_COL;
	colors[ImGuiCol_TitleBg] = TITLE_COL;
	colors[ImGuiCol_TitleBgActive] = SECONDARY_COL;
	colors[ImGuiCol_TitleBgCollapsed] = ACCENT_COL;
	colors[ImGuiCol_MenuBarBg] = TITLE_COL;
	colors[ImGuiCol_ScrollbarBg] = ACCENT_COL;
	colors[ImGuiCol_ScrollbarGrab] = SECONDARY_COL;
	colors[ImGuiCol_ScrollbarGrabHovered] = SECONDARY_COL;
	colors[ImGuiCol_ScrollbarGrabActive] = PRIMARY_COL;
	colors[ImGuiCol_CheckMark] = SECONDARY_COL;
	colors[ImGuiCol_SliderGrab] = ACCENT_COL;
	colors[ImGuiCol_SliderGrabActive] = ACCENT_COL;
	colors[ImGuiCol_Button] = SECONDARY_COL;
	colors[ImGuiCol_ButtonHovered] = PRIMARY_COL;
	colors[ImGuiCol_ButtonActive] = ACCENT_COL;
	colors[ImGuiCol_Header] = ACCENT_COL;
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_Separator] = WHITE;
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