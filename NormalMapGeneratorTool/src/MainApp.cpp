#include <iostream>
#include <string>
#include <vector>
#include <GL\glew.h>
#include <GLFW/glfw3.h>

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
#include "WindowSystem.h"
#include "WindowTransformUtility.h"
#include "FileExplorer.h"
#include "ModelObject.h"

#include "stb_image_write.h"
#include "MeshLoadingSystem.h"

//TODO : Drawing should take copy of entire image before button press and make changes on that.(prevents overwrite)
//TODO : Add detail value in normal settings, Sampling rate in shader
//TODO : Make camera move around instead of model
//TODO : Implement modal dialouges
//TODO : Rotation editor values
//TODO : Distance based drawing
//TODO : Add custom path for preview texture on model
//TODO : Add custom theme capability (with json support)
//TODO : Undo/Redo Capability, 20 steps in RAM after that Write to disk
//TODO : Custom Brush Support
//TODO : Add cube map in preview

/*Define For Enabling Custom Window Chrome*/
//#define NORA_CUSTOM_WINDOW_CHROME

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

const int WINDOW_SIZE_MIN = 480;

FrameBufferSystem fbs;
FrameBufferSystem previewFbs;

enum class LoadingOption
{
	MODEL, TEXTURE, NONE
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) noexcept;
void CustomColourImGuiTheme(ImGuiStyle* dst = (ImGuiStyle*)0);
bool isKeyPressed(int key);
bool isKeyReleased(int key);
void SetPixelValues(TextureData& texData, int startX, int width, int startY, int height, double xpos, double ypos, const BrushData& brushData);
void SetBrushPixelValues(TextureData& inputTexData, int startX, int width, int startY, int height, double xpos, double ypos, const BrushData& brushData);
void SetBluredPixelValues(TextureData& inputTexData, int startX, int width, int startY, int height, double xpos, double ypos, const BrushData& brushData);
void HandleMiddleMouseButtonInput(int state, glm::vec2 &prevMiddleMouseButtonCoord, double deltaTime, DrawingPanel &normalmapPanel);
void HandleLeftMouseButtonInput_UI(int state, glm::vec2 &initPos, WindowSide &windowSideAtInitPos, double x, double y, bool &isMaximized, glm::vec2 &prevGlobalFirstMouseCoord);
void HandleLeftMouseButtonInput_NormalMapInteraction(int state, glm::vec2 &prevMouseCoord, BrushData &brushData, DrawingPanel &normalmapPanel, bool isBlurOn);
void SaveNormalMapToFile(const std::string &locationStr);
inline void WindowTopBarDisplay(unsigned int minimizeTexture, unsigned int restoreTexture, bool &isMaximized, unsigned int closeTexture);
inline void DisplayPreview(bool * p_open, const ImGuiWindowFlags &window_flags, int &modelViewMode, glm::vec3 &diffuseColour, glm::vec3 &ambientColour, glm::vec3 &lightColour);
inline void DisplayLightSettingsUserInterface(float &lightIntensity, float &specularity, float &specularityStrength, glm::vec3 &lightDirection);
inline void DisplayNormalSettingsUserInterface(float &normalMapStrength, bool &flipX_Ydir, bool &redChannelActive, bool &greenChannelActive, bool &blueChannelActive);
inline void DisplayBrushSettingsUserInterface(bool &isBlurOn, BrushData &brushData);
inline void HandleKeyboardInput(float &normalMapStrength, double deltaTime, int &mapDrawViewMode, DrawingPanel &frameDrawingPanel, bool &isMaximized);
inline void BottomBarDisplay(bool * p_open, const ImGuiWindowFlags &window_flags);
inline void SideBarDisplay(bool * p_open, const ImGuiWindowFlags &window_flags, DrawingPanel &frameDrawingPanel, char  imageLoadLocation[500], char  saveLocation[500], bool &shouldSaveNormalMap, bool &changeSize, int &mapDrawViewMode);
void SetStatesForSavingNormalMap(bool &changeSize, glm::vec2 &prevWindowSize, int &retflag);
void SetupImGui();

float zoomLevel = 1;
float modelPreviewRotationSpeed = 0.1f;
float modelPreviewZoomLevel = -5.0f;
bool isUsingCustomTheme = false;

TextureData texData;
MeshLoadingSystem::MeshLoader modelLoader;
ModelObject *modelPreviewObj = nullptr;
LoadingOption currentLoadingOption = LoadingOption::NONE;
FileExplorer fileExplorer;
WindowSystem windowSys;
std::string prevPath;
std::string path;

int main(void)
{
	windowSys.Init("Nora Normal Map Editor v0.85 alpha", 1600, 800);
	if (glewInit() != GLEW_OK)
	{
		std::cout << "Open GL init error" << std::endl;
		return EXIT_FAILURE;
	}
	glewExperimental = GL_TRUE;

	SetupImGui();
	glfwSetFramebufferSizeCallback((GLFWwindow*)windowSys.GetWindow(), framebuffer_size_callback);
	glfwSetScrollCallback((GLFWwindow*)windowSys.GetWindow(), scroll_callback);
	modelPreviewObj = modelLoader.CreateModelFromFile(CUBE_MODEL_PATH); // Default loaded model in preview window
	ModelObject* cubeForSkybox = modelLoader.CreateModelFromFile(CUBE_MODEL_PATH);

	DrawingPanel normalmapPanel;
	normalmapPanel.init(1.0f, 1.0f);
	DrawingPanel frameDrawingPanel;
	frameDrawingPanel.init(1.0f, 1.0f);
	DrawingPanel previewFrameDrawingPanel;
	previewFrameDrawingPanel.init(1.0f, 1.0f);
	DrawingPanel brushPanel;
	brushPanel.init(1.0f, 1.0f);

	unsigned int closeTextureId = TextureManager::loadTextureFromFile("Resources\\UI\\closeIcon.png", false);
	unsigned int restoreTextureId = TextureManager::loadTextureFromFile("Resources\\UI\\maxWinIcon.png", false);
	unsigned int minimizeTextureId = TextureManager::loadTextureFromFile("Resources\\UI\\toTrayIcon.png", false);
	unsigned int logoTextureId = TextureManager::loadTextureFromFile("Resources\\UI\\icon.png", false);
	unsigned int crateTextureId = TextureManager::loadTextureFromFile("Resources\\crate.jpg", false);

	std::vector<std::string> cubeMapImagePaths;
	cubeMapImagePaths.push_back("Resources\\Cubemap Textures\\Sahara Desert Cubemap\\sahara_bk.tga");
	cubeMapImagePaths.push_back("Resources\\Cubemap Textures\\Sahara Desert Cubemap\\sahara_dn.tga");
	cubeMapImagePaths.push_back("Resources\\Cubemap Textures\\Sahara Desert Cubemap\\sahara_ft.tga");
	cubeMapImagePaths.push_back("Resources\\Cubemap Textures\\Sahara Desert Cubemap\\sahara_lf.tga");
	cubeMapImagePaths.push_back("Resources\\Cubemap Textures\\Sahara Desert Cubemap\\sahara_rt.tga");
	cubeMapImagePaths.push_back("Resources\\Cubemap Textures\\Sahara Desert Cubemap\\sahara_up.tga");

	unsigned int cubeMapTextureId = TextureManager::loadCubemapFromFile(cubeMapImagePaths, false);

	TextureManager::getTextureDataFromFile("Resources\\Textures\\goli.png", texData);
	unsigned int heightmapTexId = TextureManager::loadTextureFromData(texData, false);
	normalmapPanel.setTextureID(heightmapTexId);

	ShaderProgram normalmapShader;
	normalmapShader.compileShaders("Resources\\Shaders\\normalPanel.vs", "Resources\\Shaders\\normalPanel.fs");
	normalmapShader.linkShaders();

	ShaderProgram modelViewShader;
	modelViewShader.compileShaders("Resources\\Shaders\\modelView.vs", "Resources\\Shaders\\modelView.fs");
	modelViewShader.linkShaders();

	ShaderProgram frameShader;
	frameShader.compileShaders("Resources\\Shaders\\normalPanel.vs", "Resources\\Shaders\\frameBuffer.fs");
	frameShader.linkShaders();

	ShaderProgram brushPreviewShader;
	brushPreviewShader.compileShaders("Resources\\Shaders\\normalPanel.vs", "Resources\\Shaders\\brushPreview.fs");
	brushPreviewShader.linkShaders();

	Camera camera;
	camera.init(windowSys.GetWindowRes().x, windowSys.GetWindowRes().y);

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
	int modelCubeMapTextureUniform = modelViewShader.getUniformLocation("skybox");


	float normalMapStrength = 10.0f;
	float specularity = 10.0f;
	float specularityStrength = 0.5f;
	float lightIntensity = 0.5f;
	glm::vec3 lightDirection = glm::vec3(90.0f, 90.0f, 60.0f);
	zoomLevel = 1;
	int mapDrawViewMode = 1;
	int modelViewMode = 1;
	bool flipX_Ydir = false;
	bool redChannelActive = true;
	bool greenChannelActive = true;
	bool blueChannelActive = true;

	glm::vec3 rotation = glm::vec3(0);
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

	fbs.init(windowSys.GetWindowRes());
	previewFbs.init(windowSys.GetWindowRes());

	glm::vec2 prevMouseCoord = glm::vec2(-10, -10);
	glm::vec2 prevMiddleMouseButtonCoord = glm::vec2(-10, -10);
	glm::vec2 prevGlobalFirstMouseCoord = glm::vec2(-500, -500);

	bool shouldSaveNormalMap = false;
	bool changeSize = false;
	glm::vec2  prevWindowSize = glm::vec2(500, 500);

	double initTime = glfwGetTime();
	while (!windowSys.IsWindowClosing())
	{
		const double deltaTime = glfwGetTime() - initTime;
		initTime = glfwGetTime();

		glViewport(0, 0, windowSys.GetWindowRes().x, windowSys.GetWindowRes().y);
		if (shouldSaveNormalMap)
		{
			int retflag;
			SetStatesForSavingNormalMap(changeSize, prevWindowSize, retflag);
			if (retflag == 3) continue;
		}
		static glm::vec2 initPos = glm::vec2(-1000, -1000);
		static WindowSide windowSideAtInitPos = WindowSide::NONE;

		const glm::vec2 curPos = windowSys.GetCursorPos();
		HandleKeyboardInput(normalMapStrength, deltaTime, mapDrawViewMode, frameDrawingPanel, isMaximized);

		fbs.BindFrameBuffer();
		glClearColor(0.9f, 0.5f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		glBindTexture(GL_TEXTURE_2D, heightmapTexId);
		normalmapShader.use();

		const WindowSide currentMouseCoordWindowSide = WindowTransformUtility::GetWindowSideAtMouseCoord(curPos, windowSys.GetWindowRes());
		if (windowSideAtInitPos == WindowSide::LEFT || windowSideAtInitPos == WindowSide::RIGHT || currentMouseCoordWindowSide == WindowSide::LEFT || currentMouseCoordWindowSide == WindowSide::RIGHT)
			ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
		else if (windowSideAtInitPos == WindowSide::TOP || windowSideAtInitPos == WindowSide::BOTTOM || currentMouseCoordWindowSide == WindowSide::TOP || currentMouseCoordWindowSide == WindowSide::BOTTOM)
			ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
		else
			ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNWSE);

		//---- Making sure the dimensions do not change for drawing panel ----//
		const float aspectRatio = windowSys.GetAspectRatio();
		glm::vec2 aspectRatioHolder;
		if (windowSys.GetWindowRes().x < windowSys.GetWindowRes().y)
			aspectRatioHolder = glm::vec2(1, aspectRatio);
		else
			aspectRatioHolder = glm::vec2(1.0f / aspectRatio, 1);
		frameDrawingPanel.getTransform()->setScale(aspectRatioHolder * zoomLevel);
		frameDrawingPanel.getTransform()->setX(glm::clamp(frameDrawingPanel.getTransform()->getPosition().x, -0.5f, 0.5f));
		frameDrawingPanel.getTransform()->setY(glm::clamp(frameDrawingPanel.getTransform()->getPosition().y, -1.0f, 1.0f));
		frameDrawingPanel.getTransform()->update();

		const int leftMouseButtonState = glfwGetMouseButton((GLFWwindow*)windowSys.GetWindow(), GLFW_MOUSE_BUTTON_LEFT);
		const int middleMouseButtonState = glfwGetMouseButton((GLFWwindow*)windowSys.GetWindow(), GLFW_MOUSE_BUTTON_MIDDLE);

		HandleLeftMouseButtonInput_UI(leftMouseButtonState, initPos, windowSideAtInitPos, curPos.x, curPos.y, isMaximized, prevGlobalFirstMouseCoord);
		HandleLeftMouseButtonInput_NormalMapInteraction(leftMouseButtonState, prevMouseCoord, brushData, frameDrawingPanel, isBlurOn);
		HandleMiddleMouseButtonInput(middleMouseButtonState, prevMiddleMouseButtonCoord, deltaTime, frameDrawingPanel);

		normalmapPanel.getTransform()->update();
		//---- Applying Normal Map Shader Uniforms---//
		normalmapShader.applyShaderUniformMatrix(normalPanelModelMatrixUniform, normalmapPanel.getTransform()->getMatrix());
		normalmapShader.applyShaderFloat(strengthValueUniform, normalMapStrength);
		normalmapShader.applyShaderFloat(specularityUniform, specularity);
		normalmapShader.applyShaderFloat(specularityStrengthUniform, specularityStrength);
		normalmapShader.applyShaderFloat(lightIntensityUniform, lightIntensity);
		normalmapShader.applyShaderVector3(lightDirectionUniform, glm::normalize(glm::vec3(lightDirection.x / 180.0f, lightDirection.y / 180.0f, lightDirection.z / 180.0f)));
		normalmapShader.applyShaderFloat(widthUniform, texData.getWidth());
		normalmapShader.applyShaderFloat(heightUniform, texData.getHeight());
		normalmapShader.applyShaderInt(normalMapModeOnUniform, mapDrawViewMode);
		normalmapShader.applyShaderBool(flipXYdirUniform, flipX_Ydir);
		normalmapShader.applyShaderBool(RedChannelUniform, redChannelActive);
		normalmapShader.applyShaderBool(GreenChannelUniform, greenChannelActive);
		normalmapShader.applyShaderBool(BlueChannelUniform, blueChannelActive);
		normalmapPanel.draw();

		static char saveLocation[500] = "D:\\scr.tga";
		static char imageLoadLocation[500] = "Resources\\Textures\\goli.png";
		if (shouldSaveNormalMap)
		{
			SaveNormalMapToFile(saveLocation);
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
		modelViewShader.applyShaderInt(modelCubeMapTextureUniform, 2);

		modelViewShader.applyShaderVector3(modelDiffuseColourUniform, diffuseColour);
		modelViewShader.applyShaderVector3(modelLightColourUniform, lightColour);
		modelViewShader.applyShaderVector3(modelAmbientColourUniform, ambientColour);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, heightmapTexId);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, crateTextureId);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTextureId);
		if (modelPreviewObj != nullptr)
			modelPreviewObj->draw();
		glActiveTexture(GL_TEXTURE0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		if (windowSys.GetWindowRes().x < windowSys.GetWindowRes().y)
		{
			float scale = 1.0f;
			brushPanel.getTransform()->setScale(glm::vec2((brushData.brushScale / texData.getWidth())*scale, (brushData.brushScale / texData.getHeight())*aspectRatio) * zoomLevel * 2.0f);
		}
		else
		{
			float scale = 1.0f;
			if (windowSys.GetWindowRes().y < texData.getHeight())
				scale = 1;
			brushPanel.getTransform()->setScale(glm::vec2((brushData.brushScale / texData.getWidth()) / aspectRatio, (brushData.brushScale / texData.getHeight())*scale) * zoomLevel * 2.0f);
		}
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		brushPreviewShader.use();
		brushPanel.getTransform()->setPosition(((curPos.x / windowSys.GetWindowRes().x)*2.0f) - 1.0f,
			-(((curPos.y / windowSys.GetWindowRes().y)*2.0f) - 1.0f));
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
		bool *p_open = nullptr;

		BottomBarDisplay(p_open, window_flags);
		SideBarDisplay(p_open, window_flags, frameDrawingPanel, imageLoadLocation,
			saveLocation, shouldSaveNormalMap, changeSize, mapDrawViewMode);
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
			for (unsigned int i = 0; i < path.length(); i++)
				imageLoadLocation[i] = path[i];
			if (currentLoadingOption == LoadingOption::TEXTURE)
			{
				TextureManager::getTextureDataFromFile(path, texData);
				heightmapTexId = TextureManager::loadTextureFromData(texData, false);
				normalmapPanel.setTextureID(heightmapTexId);
			}
			else if (currentLoadingOption == LoadingOption::MODEL)
			{
				modelPreviewObj = modelLoader.CreateModelFromFile(path);
			}
			currentLoadingOption = LoadingOption::NONE;
			prevPath = path;
		}
		windowSys.UpdateWindow();
	}

	ImGui_ImplOpenGL2_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	windowSys.Close();
	return 0;
}
inline void SideBarDisplay(bool * p_open, const ImGuiWindowFlags &window_flags, DrawingPanel &frameDrawingPanel, char  imageLoadLocation[500], char  saveLocation[500], bool &shouldSaveNormalMap, bool &changeSize, int &mapDrawViewMode)
{
	ImGui::SetNextWindowPos(ImVec2(windowSys.GetWindowRes().x - 5, 42), ImGuiSetCond_Always);
	ImGui::SetNextWindowSize(ImVec2(10, windowSys.GetWindowRes().y - 67), ImGuiSetCond_Always);
	ImGui::Begin("Side_Bar", p_open, window_flags);
	ImGui::End();

	ImGui::SetNextWindowPos(ImVec2(0, 42), ImGuiSetCond_Always);
	ImGui::SetNextWindowSize(ImVec2(glm::clamp(windowSys.GetWindowRes().x * 0.15f, 280.0f, 600.0f), windowSys.GetWindowRes().y - 67), ImGuiSetCond_Always);
	ImGui::Begin("Settings", p_open, window_flags);
	ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth() * 0.5f);

	if (ImGui::Button("Toggle Fullscreen", ImVec2(ImGui::GetContentRegionAvailWidth(), 40)))
	{
		if (!windowSys.IsFullscreen())
			windowSys.SetFullscreen(true);
		else
			windowSys.SetFullscreen(false);
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
		currentLoadingOption = LoadingOption::TEXTURE;
		fileExplorer.displayDialog(&path, FileType::IMAGE);
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
	ImGui::SetNextWindowPos(ImVec2(0, windowSys.GetWindowRes().y - 25), ImGuiSetCond_Always);
	ImGui::SetNextWindowSize(ImVec2(windowSys.GetWindowRes().x, 25), ImGuiSetCond_Always);
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
	ImGui_ImplGlfw_InitForOpenGL((GLFWwindow*)windowSys.GetWindow(), true);
	ImGui_ImplOpenGL2_Init();
	// Setup ImGui Theme
	CustomColourImGuiTheme();
	ImFont* font = io.Fonts->AddFontFromFileTTF("Resources\\Fonts\\arial.ttf", 16.0f);
	IM_ASSERT(font != NULL);
}
void SetStatesForSavingNormalMap(bool &changeSize, glm::vec2 &prevWindowSize, int &retflag)
{
	retflag = 1;
	glViewport(0, 0, texData.getWidth(), texData.getHeight());
	fbs.updateTextureDimensions(texData.getWidth(), texData.getHeight());
	if (changeSize)
	{
		prevWindowSize = windowSys.GetWindowRes();
		changeSize = false;
		retflag = 3;
		return;
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
		windowSys.SetWindowRes(1600, 800);
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
	const float width = ImGui::GetContentRegionAvailWidth() / 3.0f - 7;

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
	ImGui::SetNextWindowPos(ImVec2(windowSys.GetWindowRes().x - 305, 42), ImGuiSetCond_Always);
	ImGui::SetNextWindowSize(ImVec2(300, windowSys.GetWindowRes().y - 67), ImGuiSetCond_Always);
	ImGui::Begin("Preview_Bar", p_open, window_flags);
	ImGui::Image((ImTextureID)previewFbs.getBufferTexture(), ImVec2(300, 300));

	const char* items[] = { "CUBE", "CYLINDER", "SPHERE", "TORUS", "CUSTOM MODEL" };
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
				case 4:
					currentLoadingOption = LoadingOption::MODEL;
					fileExplorer.displayDialog(&path, FileType::MODEL);
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
#ifdef NORA_CUSTOM_WINDOW_CHROME
		ImGui::Indent(windowWidth - 160);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(5, 10));
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		if (isUsingCustomTheme)
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ACCENT_COL);
		if (ImGui::ImageButton((ImTextureID)minimizeTexture, ImVec2(30, 30), ImVec2(0, 0), ImVec2(1, 1), 5)) { glfwIconifyWindow(window); }
		if (ImGui::ImageButton((ImTextureID)restoreTexture, ImVec2(30, 30), ImVec2(0, 0), ImVec2(1, 1), 5))
		{
			if (!isFullscreen)
				glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, videoMode->width, videoMode->height, 60);
			else
				glfwSetWindowMonitor(window, NULL, 100, 100, (videoMode->width / 1.2f), (videoMode->height / 1.2f), 60);
			isFullscreen = !isFullscreen;
		}
		if (ImGui::ImageButton((ImTextureID)closeTexture, ImVec2(30, 30), ImVec2(0, 0), ImVec2(1, 1), 5)) { glfwSetWindowShouldClose(window, true); }
		ImGui::PopStyleColor();
		if (isUsingCustomTheme)
			ImGui::PopStyleColor();
		ImGui::PopStyleVar();
#endif
	}
	ImGui::EndMainMenuBar();
	ImGui::PopStyleVar();
}
void SaveNormalMapToFile(const std::string &locationStr)
{
	if (locationStr.length() > 4)
	{
		fbs.BindBufferTexture();
		glPixelStorei(GL_PACK_ALIGNMENT, 1);

		const int nSize = texData.getWidth() * texData.getHeight() * 3;
		char* dataBuffer = new char[nSize];
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, dataBuffer);

		glBindTexture(GL_TEXTURE_2D, 0);
		fbs.updateTextureDimensions(windowSys.GetWindowRes().x, windowSys.GetWindowRes().y);
		stbi_write_bmp(locationStr.c_str(), texData.getWidth(), texData.getHeight(), 3, dataBuffer);
		std::cout << "Saved at " << locationStr;
		delete dataBuffer;
	}
}
inline void HandleLeftMouseButtonInput_NormalMapInteraction(int state, glm::vec2 &prevMouseCoord, BrushData &brushData, DrawingPanel &frameBufferPanel, bool isBlurOn)
{
	if (state == GLFW_PRESS)
	{
		double xpos, ypos;
		glm::vec2 currentMouseCoord = windowSys.GetCursorPos();
		xpos = currentMouseCoord.x;
		ypos = currentMouseCoord.y;

		glm::vec2 windowNormalizedCurrentMouseCoord = glm::vec2(currentMouseCoord.x / windowSys.GetWindowRes().x, currentMouseCoord.y / windowSys.GetWindowRes().y);
		glm::vec2 windowNormalizedPrevMouseCoord = glm::vec2(prevMouseCoord.x / windowSys.GetWindowRes().x, prevMouseCoord.y / windowSys.GetWindowRes().y);
		float minDistThresholdForDraw = (brushData.brushRate / texData.getWidth()) * zoomLevel;
		float distOfPrevAndCurrentMouseCoord = glm::distance(windowNormalizedCurrentMouseCoord, windowNormalizedPrevMouseCoord);

		if (currentMouseCoord != prevMouseCoord && distOfPrevAndCurrentMouseCoord > minDistThresholdForDraw)
		{
			xpos = xpos / windowSys.GetWindowRes().x; // window size normalized width
			ypos = 1.0f - (ypos / windowSys.GetWindowRes().y); // window size normalized height

			//std::cout << "\nCursor pos : " << xpos << ", " << ypos;
			/*glm::vec4 wD = frameBufferPanel.getPanelWorldDimension();
			std::cout << "\nLeft side loc bound : " << wD.x;*/
			float leftBound = (frameBufferPanel.getTransform()->getPosition().x + 1) * 0.5f - 0.25f;
			float rightBound = (frameBufferPanel.getTransform()->getPosition().x + 1) * 0.5f + 0.25f;
			float topBound = (frameBufferPanel.getTransform()->getPosition().y + 1) * 0.5f + 0.25f;
			float bottomBound = (frameBufferPanel.getTransform()->getPosition().y + 1) * 0.5f - 0.25f;
			//std::cout << "\nLeft bound = " << leftBound << " Right bound = " << rightBound
				//<< " Top bound = " << topBound << " Bottom bound = " << bottomBound;

			if (xpos >= leftBound && xpos <= rightBound /*&& ypos >= bottomBound && ypos <= topBound*//*frameBufferPanel.isPointInPanel(xpos, ypos)*/) //not working correctly
			{
				float prevX = prevMouseCoord.x / windowSys.GetWindowRes().x;
				float prevY = 1.0f - (prevMouseCoord.y / windowSys.GetWindowRes().y);

				glm::vec4 worldDimensions = frameBufferPanel.getPanelWorldDimension();

				xpos = /*(xpos - leftBound) / (rightBound - leftBound);*/((xpos - worldDimensions.x) / (worldDimensions.y - worldDimensions.x)) + (frameBufferPanel.getTransform()->getPosition().x * zoomLevel * 0.5f); //works at default zoom as 0.5
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
						prevX = /*(prevX - leftBound) / (rightBound - leftBound);*/((prevX - worldDimensions.x) / (worldDimensions.y - worldDimensions.x)) + (frameBufferPanel.getTransform()->getPosition().x * zoomLevel * 0.5f); //works at default zoom as 0.5
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
			windowSideAtInitPos = WindowTransformUtility::GetWindowSideAtMouseCoord((int)x, (int)y, windowSys.GetWindowRes().x, windowSys.GetWindowRes().y);
			initPos = glm::vec2(x, y);
		}

		if (y < 40 && y >= WindowTransformUtility::BORDER_SIZE)
		{
			if (windowSideAtInitPos == WindowSide::NONE)
			{
				glm::vec2 currentPos(x, y);
				if (prevGlobalFirstMouseCoord != currentPos && prevGlobalFirstMouseCoord != glm::vec2(-500, -500))
				{
					glm::vec2 winPos = windowSys.GetWindowPos();
					windowSys.SetWindowPos(winPos.x + currentPos.x - initPos.x, winPos.y + currentPos.y - initPos.y);
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
				glm::vec2 winPos = windowSys.GetWindowPos();
				windowSys.SetWindowPos(winPos.x + currentPos.x, winPos.y);
				const glm::vec2 diff = (currentPos + glm::vec2(winPos.x, 0)) - (initPos + glm::vec2(winPos.x, 0));
				windowSys.SetWindowRes(windowSys.GetWindowRes().x - diff.x, windowSys.GetWindowRes().y);
			}
		}
		else if (windowSideAtInitPos == WindowSide::RIGHT)
		{
			glm::vec2 currentPos(x, y);
			if (initPos != currentPos && initPos != glm::vec2(-1000, -1000))
			{
				glm::vec2 diff = currentPos - initPos;
				windowSys.SetWindowRes(windowSys.GetWindowRes().x + diff.x, windowSys.GetWindowRes().y);
			}
		}
		else if (windowSideAtInitPos == WindowSide::BOTTOM_RIGHT)
		{
			glm::vec2 currentPos(x, y);
			if (initPos != currentPos && initPos != glm::vec2(-1000, -1000))
			{
				glm::vec2 diff = currentPos - initPos;
				windowSys.SetWindowRes(windowSys.GetWindowRes() + diff);
			}
		}
		else if (windowSideAtInitPos == WindowSide::TOP)
		{
			glm::vec2 currentPos(x, y);
			if (initPos != currentPos && initPos != glm::vec2(-1000, -1000))
			{
				glm::vec2 diff = currentPos - initPos;
				windowSys.SetWindowRes(windowSys.GetWindowRes().x, windowSys.GetWindowRes().y - diff.y);
				glm::vec2 winPos = windowSys.GetWindowPos();
				windowSys.SetWindowPos(winPos.x + currentPos.x - initPos.x, winPos.y + currentPos.y - initPos.y);
			}
		}
		else if (windowSideAtInitPos == WindowSide::BOTTOM)
		{
			glm::vec2 currentPos(x, y);
			if (initPos != currentPos && initPos != glm::vec2(-1000, -1000))
			{
				const glm::vec2 diff = currentPos - initPos;
				windowSys.SetWindowRes(windowSys.GetWindowRes().x, windowSys.GetWindowRes().y + diff.y);
			}
		}
		else if (windowSideAtInitPos == WindowSide::BOTTOM_LEFT)
		{
			glm::vec2 currentPos(x, y);
			if (initPos != currentPos && initPos != glm::vec2(-1000, -1000))
			{
				glm::vec2 diff = currentPos - initPos;
				windowSys.SetWindowRes(windowSys.GetWindowRes().x - diff.x, windowSys.GetWindowRes().y + diff.y);
				glm::vec2 winPos = windowSys.GetWindowPos();
				windowSys.SetWindowPos(winPos.x + currentPos.x, winPos.y);
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
		glm::vec2 currentPos = windowSys.GetCursorPos();
		glm::vec2 diff = (currentPos - prevMiddleMouseButtonCoord) * glm::vec2(1.0f / windowSys.GetWindowRes().x, 1.0f / windowSys.GetWindowRes().y) * 2.0f;
		frameBufferPanel.getTransform()->translate(diff.x, -diff.y);
		prevMiddleMouseButtonCoord = currentPos;
	}
	else
	{
		prevMiddleMouseButtonCoord = windowSys.GetCursorPos();
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
	float distance;
	glm::vec2 pixelPos(xpos, ypos);
	const float px_width = inputTexData.getWidth();
	const float px_height = inputTexData.getHeight();
	const float distanceRemap = brushData.brushScale / px_height;

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
	float distance = 0;
	const glm::vec2 pixelPos(xpos, ypos);
	const glm::vec2 texRes = glm::vec2(inputTexData.getWidth(), inputTexData.getHeight());
	const float distanceRemap = 0.5f;
	for (int i = startX; i < width; i++)
	{
		for (int j = startY; j < height; j++)
		{
			distance = glm::distance(pixelPos, texRes);
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
	int state = glfwGetKey((GLFWwindow*)windowSys.GetWindow(), key);
	if (state == GLFW_PRESS)
		return true;
	return false;
}

bool isKeyReleased(int key)
{
	int state = glfwGetKey((GLFWwindow*)windowSys.GetWindow(), key);
	if (state == GLFW_RELEASE)
		return true;
	return false;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	width = glm::clamp(width, WINDOW_SIZE_MIN, (int)windowSys.GetMaxWindowRes().x);
	height = glm::clamp(height, WINDOW_SIZE_MIN, (int)windowSys.GetMaxWindowRes().y);

	windowSys.SetWindowRes(width, height);
	fbs.updateTextureDimensions(windowSys.GetWindowRes().x, windowSys.GetWindowRes().y);
	previewFbs.updateTextureDimensions(windowSys.GetWindowRes().x, windowSys.GetWindowRes().y);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) noexcept
{
	zoomLevel += zoomLevel * 0.1f * yoffset;
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