#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <queue>
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
#include "ThemeManager.h"
#include "ModalWindow.h"

#include "stb_image_write.h"
#include "UndoRedoSystem.h"
#include "MeshLoadingSystem.h"

//TODO : Rotation editor values
//TODO : Better Blurring
//TODO : Add custom theme capability (with json support)
//TODO : Custom Brush Support
//TODO : Implement mouse position record and draw to prevent cursor skipping ( probably need separate thread for drawing |completly async| )
//TODO : Filters added with file explorer

enum class LoadingOption
{
	MODEL, TEXTURE, NONE
};

const std::string VERSION_NAME = "v0.90 Alpha";
const std::string FONTS_PATH = "Resources\\Fonts\\";
const std::string TEXTURES_PATH = "Resources\\Textures\\";
const std::string CUBEMAP_TEXTURES_PATH = "Resources\\Cubemap Textures\\";
const std::string UI_TEXTURES_PATH = "Resources\\UI\\";
const std::string SHADERS_PATH = "Resources\\Shaders\\";
const std::string MODELS_PATH = "Resources\\3D Models\\Primitives\\";
const std::string CUBE_MODEL_PATH = MODELS_PATH + "Cube.obj";
const std::string CYLINDER_MODEL_PATH = MODELS_PATH + "Cylinder.obj";
const std::string SPHERE_MODEL_PATH = MODELS_PATH + "Sphere.obj";
const std::string TORUS_MODEL_PATH = MODELS_PATH + "Torus.obj";

const int WINDOW_SIZE_MIN = 480;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) noexcept;
bool isKeyPressed(int key);
bool isKeyReleased(int key);
bool isKeyPressedDown(int key);
void SetPixelValues(TextureData& texData, int startX, int width, int startY, int height, double xpos, double ypos);
void SetBluredPixelValues(TextureData& inputTexData, int startX, int width, int startY, int height, double xpos, double ypos);
void SaveNormalMapToFile(const std::string &locationStr);
inline void HandleMiddleMouseButtonInput(int state, glm::vec2 &prevMiddleMouseButtonCoord, double deltaTime, DrawingPanel &normalmapPanel);
inline void HandleLeftMouseButtonInput_UI(int state, glm::vec2 &initPos, WindowSide &windowSideAtInitPos, double x, double y, bool &isMaximized, glm::vec2 &prevGlobalFirstMouseCoord);
inline void HandleLeftMouseButtonInput_NormalMapInteraction(int state, glm::vec2 &prevMouseCoord, DrawingPanel &normalmapPanel, bool isBlurOn);
inline void WindowTopBarDisplay(unsigned int minimizeTexture, unsigned int restoreTexture, bool &isMaximized, unsigned int closeTexture);
inline void BottomBarDisplay(const ImGuiWindowFlags &window_flags);
inline void DisplayPreview(const ImGuiWindowFlags &window_flags, int &modelViewMode, glm::vec3 &diffuseColour, glm::vec3 &ambientColour, glm::vec3 &lightColour);
inline void DisplayLightSettingsUserInterface(float &lightIntensity, float &specularity, float &specularityStrength, glm::vec3 &lightDirection);
inline void DisplayNormalSettingsUserInterface(float &normalMapStrength, bool &flipX_Ydir, bool &redChannelActive, bool &greenChannelActive, bool &blueChannelActive);
inline void DisplayBrushSettingsUserInterface(bool &isBlurOn);
inline void HandleKeyboardInput(float &normalMapStrength, double deltaTime, int &mapDrawViewMode, DrawingPanel &frameDrawingPanel, bool &isMaximized);
inline void SideBarDisplay(const ImGuiWindowFlags &window_flags, DrawingPanel &frameDrawingPanel, char  saveLocation[500], bool &shouldSaveNormalMap, bool &changeSize, int &mapDrawViewMode);
void SetStatesForSavingNormalMap();
void SetupImGui();

float zoomLevel = 1;
float modelPreviewRotationSpeed = 0.1f;
float modelPreviewZoomLevel = -5.0f;
float modelRoughness = 5.0f;
float modelReflectivity = 0.5f;
bool isUsingCustomTheme = false;

ImFont* menuBarLargerText = NULL;
MeshLoadingSystem::MeshLoader modelLoader;

FrameBufferSystem fbs;
FrameBufferSystem previewFbs;

BrushData brushData;
TextureData heightMapTexData;
TextureData diffuseTexDataForPreview;
ModelObject *modelPreviewObj = nullptr;
LoadingOption currentLoadingOption = LoadingOption::NONE;
FileExplorer fileExplorer;
ModalWindow modalWindow;
WindowSystem windowSys;
ThemeManager themeManager;
DrawingPanel normalmapPanel;
UndoRedoSystem undoRedoSystem(512 * 512 * 4 * 20, 512 * 512 * 4);

struct BoundsAndPos
{
public:
	glm::vec2 mouseCoord;
	float left, right, bottom, up;
};

std::queue<BoundsAndPos> mouseCoordQueue;


void ApplyChangesToPanel()
{
	const float maxWidth = heightMapTexData.getWidth();
	const float convertedBrushScale = (brushData.brushScale / heightMapTexData.getHeight()) * maxWidth * 3.5f;

	while (!windowSys.IsWindowClosing())
	{
		while (mouseCoordQueue.size() > 0)
		{
			BoundsAndPos boundAndPos = mouseCoordQueue.front();
			mouseCoordQueue.pop();
			glm::vec2 mousePos = boundAndPos.mouseCoord;
			glm::vec2 prevMousePos = mouseCoordQueue.front().mouseCoord;

			glm::vec2 curPoint = prevMousePos;
			glm::vec2 incValue = (prevMousePos - mousePos) * 0.333f;
			curPoint += incValue;

			for (int i = 0; i < 3; i++)
			{
				float left = boundAndPos.left;
				float right = boundAndPos.right;
				float bottom = boundAndPos.bottom;
				float top = boundAndPos.up;

				left = glm::clamp(left, 0.0f, maxWidth);
				right = glm::clamp(right, 0.0f, maxWidth);
				bottom = glm::clamp(bottom, 0.0f, maxWidth);
				top = glm::clamp(top, 0.0f, maxWidth);

				curPoint += incValue;
				SetPixelValues(heightMapTexData, left, right, bottom, top, curPoint.x, curPoint.y);
			}
			//SetPixelValues(heightMapTexData, boundAndPos.left, boundAndPos.right, boundAndPos.bottom, boundAndPos.up, mousePos.x, mousePos.y, brushData);
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}

int main(void)
{
	windowSys.Init("Nora Normal Map Editor " + VERSION_NAME, 1600, 800);
	if (glewInit() != GLEW_OK)
	{
		std::cout << "Open GL init error" << std::endl;
		return EXIT_FAILURE;
	}
	glewExperimental = GL_TRUE;
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	SetupImGui();
	themeManager.Init();
	themeManager.EnableInBuiltTheme(ThemeManager::Theme::DEFAULT);

	glfwSetFramebufferSizeCallback((GLFWwindow*)windowSys.GetWindow(), framebuffer_size_callback);
	glfwSetScrollCallback((GLFWwindow*)windowSys.GetWindow(), scroll_callback);
	modelPreviewObj = modelLoader.CreateModelFromFile(CUBE_MODEL_PATH); // Default loaded model in preview window
	ModelObject* cubeForSkybox = modelLoader.CreateModelFromFile(CUBE_MODEL_PATH);

	normalmapPanel.init(1.0f, 1.0f);
	DrawingPanel frameDrawingPanel;
	frameDrawingPanel.init(1.0f, 1.0f);
	DrawingPanel previewFrameDrawingPanel;
	previewFrameDrawingPanel.init(1.0f, 1.0f);
	DrawingPanel brushPanel;
	brushPanel.init(1.0f, 1.0f);

	unsigned int closeTextureId = TextureManager::loadTextureFromFile(UI_TEXTURES_PATH + "closeIcon.png", false);
	unsigned int restoreTextureId = TextureManager::loadTextureFromFile(UI_TEXTURES_PATH + "maxWinIcon.png", false);
	unsigned int minimizeTextureId = TextureManager::loadTextureFromFile(UI_TEXTURES_PATH + "toTrayIcon.png", false);
	unsigned int logoTextureId = TextureManager::loadTextureFromFile(UI_TEXTURES_PATH + "icon.png", false);

	std::vector<std::string> cubeMapImagePaths;
	cubeMapImagePaths.push_back(CUBEMAP_TEXTURES_PATH + "Sahara Desert Cubemap\\sahara_lf.tga");
	cubeMapImagePaths.push_back(CUBEMAP_TEXTURES_PATH + "Sahara Desert Cubemap\\sahara_rt.tga");
	cubeMapImagePaths.push_back(CUBEMAP_TEXTURES_PATH + "Sahara Desert Cubemap\\sahara_dn.tga");
	cubeMapImagePaths.push_back(CUBEMAP_TEXTURES_PATH + "Sahara Desert Cubemap\\sahara_up.tga");
	cubeMapImagePaths.push_back(CUBEMAP_TEXTURES_PATH + "Sahara Desert Cubemap\\sahara_ft.tga");
	cubeMapImagePaths.push_back(CUBEMAP_TEXTURES_PATH + "Sahara Desert Cubemap\\sahara_bk.tga");

	unsigned int cubeMapTextureId = TextureManager::loadCubemapFromFile(cubeMapImagePaths, false);
	diffuseTexDataForPreview.SetTexId(TextureManager::loadTextureFromFile(TEXTURES_PATH + "crate.jpg", false));

	TextureManager::getTextureDataFromFile(TEXTURES_PATH + "goli.png", heightMapTexData);
	heightMapTexData.SetTexId(TextureManager::loadTextureFromData(heightMapTexData, false));
	undoRedoSystem.record(heightMapTexData.getTextureData());

	normalmapPanel.setTextureID(heightMapTexData.GetTexId());

	ShaderProgram normalmapShader;
	normalmapShader.compileShaders(SHADERS_PATH + "normalPanel.vs", SHADERS_PATH + "normalPanel.fs");
	normalmapShader.linkShaders();

	ShaderProgram modelViewShader;
	modelViewShader.compileShaders(SHADERS_PATH + "modelView.vs", SHADERS_PATH + "modelView.fs");
	modelViewShader.linkShaders();

	ShaderProgram frameShader;
	frameShader.compileShaders(SHADERS_PATH + "normalPanel.vs", SHADERS_PATH + "frameBuffer.fs");
	frameShader.linkShaders();

	ShaderProgram brushPreviewShader;
	brushPreviewShader.compileShaders(SHADERS_PATH + "normalPanel.vs", SHADERS_PATH + "brushPreview.fs");
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
	int modelCameraZoomUniform = modelViewShader.getUniformLocation("_CameraZoom");
	int modelNormalMapModeUniform = modelViewShader.getUniformLocation("_normalMapModeOn");
	int modelNormalMapStrengthUniform = modelViewShader.getUniformLocation("_HeightmapStrength");
	int modelLightIntensityUniform = modelViewShader.getUniformLocation("_LightIntensity");
	int modelLightSpecularityUniform = modelViewShader.getUniformLocation("_Specularity");
	int modelLightSpecularityStrengthUniform = modelViewShader.getUniformLocation("_SpecularStrength");
	int modelRoughnessUniform = modelViewShader.getUniformLocation("_Roughness");
	int modelReflectivityUniform = modelViewShader.getUniformLocation("_Reflectivity");
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
	int modelViewMode = 2;
	bool flipX_Ydir = false;
	bool redChannelActive = true;
	bool greenChannelActive = true;
	bool blueChannelActive = true;

	bool isMaximized = false;
	bool isBlurOn = false;

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

	//std::thread applyPanelChangeThread(ApplyChangesToPanel);
	double initTime = glfwGetTime();
	while (!windowSys.IsWindowClosing())
	{
		const double deltaTime = glfwGetTime() - initTime;
		initTime = glfwGetTime();

		glViewport(0, 0, windowSys.GetWindowRes().x, windowSys.GetWindowRes().y);
		if (shouldSaveNormalMap)
			SetStatesForSavingNormalMap();
		static glm::vec2 initPos = glm::vec2(-1000, -1000);
		static WindowSide windowSideAtInitPos = WindowSide::NONE;

		const glm::vec2 curPos = windowSys.GetCursorPos();
		HandleKeyboardInput(normalMapStrength, deltaTime, mapDrawViewMode, frameDrawingPanel, isMaximized);

		fbs.BindFrameBuffer();
		glClearColor(0.9f, 0.5f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		glBindTexture(GL_TEXTURE_2D, heightMapTexData.GetTexId());
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
		HandleLeftMouseButtonInput_NormalMapInteraction(leftMouseButtonState, prevMouseCoord, frameDrawingPanel, isBlurOn);
		HandleMiddleMouseButtonInput(middleMouseButtonState, prevMiddleMouseButtonCoord, deltaTime, frameDrawingPanel);

		heightMapTexData.updateTexture();

		normalmapPanel.getTransform()->update();
		//---- Applying Normal Map Shader Uniforms---//
		normalmapShader.applyShaderUniformMatrix(normalPanelModelMatrixUniform, normalmapPanel.getTransform()->getMatrix());
		normalmapShader.applyShaderFloat(strengthValueUniform, normalMapStrength);
		normalmapShader.applyShaderFloat(specularityUniform, specularity);
		normalmapShader.applyShaderFloat(specularityStrengthUniform, specularityStrength);
		normalmapShader.applyShaderFloat(lightIntensityUniform, lightIntensity);
		normalmapShader.applyShaderVector3(lightDirectionUniform, glm::normalize(glm::vec3(lightDirection.x / 180.0f, lightDirection.y / 180.0f, lightDirection.z / 180.0f)));
		normalmapShader.applyShaderFloat(widthUniform, heightMapTexData.getWidth());
		normalmapShader.applyShaderFloat(heightUniform, heightMapTexData.getHeight());
		normalmapShader.applyShaderInt(normalMapModeOnUniform, mapDrawViewMode);
		normalmapShader.applyShaderBool(flipXYdirUniform, flipX_Ydir);
		normalmapShader.applyShaderBool(RedChannelUniform, redChannelActive);
		normalmapShader.applyShaderBool(GreenChannelUniform, greenChannelActive);
		normalmapShader.applyShaderBool(BlueChannelUniform, blueChannelActive);
		normalmapPanel.draw();

		static char saveLocation[500] = "D:\\scr.tga";
		if (shouldSaveNormalMap)
		{
			SaveNormalMapToFile(saveLocation);
			shouldSaveNormalMap = false;
			modalWindow.setModalDialog("INFO", "Image exported to : " + std::string(saveLocation) + "\nResolution : " +
				std::to_string(heightMapTexData.getWidth()) + "x" + std::to_string(heightMapTexData.getHeight()));
			continue;
		}

		if (isKeyPressedDown(GLFW_KEY_F10))
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
		modelViewShader.applyShaderFloat(modelCameraZoomUniform, modelPreviewZoomLevel);
		modelViewShader.applyShaderFloat(modelNormalMapStrengthUniform, normalMapStrength);
		modelViewShader.applyShaderFloat(modelLightIntensityUniform, lightIntensity);
		modelViewShader.applyShaderFloat(modelLightSpecularityUniform, specularity);
		modelViewShader.applyShaderFloat(modelLightSpecularityStrengthUniform, specularityStrength);
		modelViewShader.applyShaderFloat(modelRoughnessUniform, modelRoughness);
		modelViewShader.applyShaderFloat(modelReflectivityUniform, modelReflectivity);
		modelViewShader.applyShaderVector3(modelLightDirectionUniform, glm::normalize(lightDirection));
		modelViewShader.applyShaderInt(modelHeightMapTextureUniform, 0);
		modelViewShader.applyShaderInt(modelTextureMapTextureUniform, 1);
		modelViewShader.applyShaderInt(modelCubeMapTextureUniform, 2);

		modelViewShader.applyShaderVector3(modelDiffuseColourUniform, diffuseColour);
		modelViewShader.applyShaderVector3(modelLightColourUniform, lightColour);
		modelViewShader.applyShaderVector3(modelAmbientColourUniform, ambientColour);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, heightMapTexData.GetTexId());
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, diffuseTexDataForPreview.GetTexId());
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTextureId);
		if (modelPreviewObj != nullptr)
			modelPreviewObj->draw();
		glActiveTexture(GL_TEXTURE0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		if (windowSys.GetWindowRes().x < windowSys.GetWindowRes().y)
		{
			float scale = 1.0f;
			brushPanel.getTransform()->setScale(glm::vec2((brushData.brushScale / heightMapTexData.getWidth())*scale, (brushData.brushScale / heightMapTexData.getHeight())*aspectRatio) * zoomLevel * 2.0f);
		}
		else
		{
			float scale = 1.0f;
			if (windowSys.GetWindowRes().y < heightMapTexData.getHeight())
				scale = 1;
			brushPanel.getTransform()->setScale(glm::vec2((brushData.brushScale / heightMapTexData.getWidth()) / aspectRatio, (brushData.brushScale / heightMapTexData.getHeight())*scale) * zoomLevel * 2.0f);
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

		BottomBarDisplay(window_flags);
		SideBarDisplay(window_flags, frameDrawingPanel,
			saveLocation, shouldSaveNormalMap, changeSize, mapDrawViewMode);
		DisplayBrushSettingsUserInterface(isBlurOn);

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
		DisplayPreview(window_flags, modelViewMode, diffuseColour, ambientColour, lightColour);
		fileExplorer.display();
		modalWindow.display();

		ImGui::Render();

		glBindVertexArray(0);
		glUseProgram(0);
		ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
		windowSys.UpdateWindow();
	}
	//applyPanelChangeThread.join();

	delete modelPreviewObj;
	delete cubeForSkybox;

	ImGui_ImplOpenGL2_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	windowSys.Destroy();
	return 0;
}
inline void SideBarDisplay(const ImGuiWindowFlags &window_flags, DrawingPanel &frameDrawingPanel, char saveLocation[500], bool &shouldSaveNormalMap, bool &changeSize, int &mapDrawViewMode)
{
	bool open = true;
	static char imageLoadLocation[500] = "Resources\\Textures\\goli.png";

	ImGui::SetNextWindowPos(ImVec2(windowSys.GetWindowRes().x - 5, 42), ImGuiSetCond_Always);
	ImGui::SetNextWindowSize(ImVec2(10, windowSys.GetWindowRes().y - 67), ImGuiSetCond_Always);
	ImGui::Begin("Side_Bar", &open, window_flags);
	ImGui::End();

	ImGui::SetNextWindowPos(ImVec2(0, 42), ImGuiSetCond_Always);
	ImGui::SetNextWindowSize(ImVec2(glm::clamp(windowSys.GetWindowRes().x * 0.15f, 280.0f, 600.0f), windowSys.GetWindowRes().y - 67), ImGuiSetCond_Always);
	ImGui::Begin("Settings", &open, window_flags);
	ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth() * 0.5f);

	ImGui::PushStyleColor(ImGuiCol_Button, themeManager.SecondaryColour);
	if (ImGui::Button("Toggle Fullscreen", ImVec2(ImGui::GetContentRegionAvailWidth(), 40)))
	{
		if (!windowSys.IsFullscreen())
			windowSys.SetFullscreen(true);
		else
			windowSys.SetFullscreen(false);
	}
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("(Press T)");
	if (ImGui::Button("Reset View", ImVec2(ImGui::GetContentRegionAvailWidth(), 40)))
	{
		frameDrawingPanel.getTransform()->setPosition(0, 0);
		zoomLevel = 1;
	}
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Reset position and scale of panel (Press R)");

	ImGui::Spacing();
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 5));
	ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth() / 1.45f);
	ImGui::InputText("## Load location", imageLoadLocation, sizeof(imageLoadLocation));
	ImGui::PopItemWidth();
	ImGui::SameLine(0, 5);
	if (ImGui::Button("LOAD", ImVec2(ImGui::GetContentRegionAvailWidth(), 27)))
	{
		currentLoadingOption = LoadingOption::TEXTURE;
		fileExplorer.displayDialog(FileType::IMAGE, [&](std::string str)
		{
			for (unsigned int i = 0; i < str.length(); i++)
				imageLoadLocation[i] = str[i];
			if (currentLoadingOption == LoadingOption::TEXTURE)
			{
				TextureManager::getTextureDataFromFile(str, heightMapTexData);
				heightMapTexData.SetTexId(TextureManager::loadTextureFromData(heightMapTexData, false));
				normalmapPanel.setTextureID(heightMapTexData.GetTexId());
			}
		});
	}
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Load grey-scale height map");
	ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth() / 1.45f);
	ImGui::InputText("## Save location", saveLocation, 500);
	ImGui::PopItemWidth();
	ImGui::SameLine(0, 5);
	if (ImGui::Button("EXPORT", ImVec2(ImGui::GetContentRegionAvailWidth(), 27))) { shouldSaveNormalMap = true; changeSize = true; }
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Save current view-mode image to file (F10)");
	ImGui::PopStyleVar();
	ImGui::PopStyleColor();
	ImGui::Spacing();
	ImGui::Text("VIEW MODE");
	ImGui::Separator();
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 10));
	int modeButtonWidth = (int)(ImGui::GetContentRegionAvailWidth() / 3.0f);
	ImGui::Spacing();

	if (mapDrawViewMode == 3) ImGui::PushStyleColor(ImGuiCol_Button, themeManager.AccentColour1);
	else ImGui::PushStyleColor(ImGuiCol_Button, themeManager.SecondaryColour);
	if (ImGui::Button("Height", ImVec2(modeButtonWidth - 5, 40))) { mapDrawViewMode = 3; }
	ImGui::PopStyleColor();
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("(Press J)");

	ImGui::SameLine(0, 5);
	if (mapDrawViewMode == 1) ImGui::PushStyleColor(ImGuiCol_Button, themeManager.AccentColour1);
	else ImGui::PushStyleColor(ImGuiCol_Button, themeManager.SecondaryColour);
	if (ImGui::Button("Normal", ImVec2(modeButtonWidth - 5, 40))) { mapDrawViewMode = 1; }
	ImGui::PopStyleColor();
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("(Press K)");

	ImGui::SameLine(0, 5);
	if (mapDrawViewMode == 2) ImGui::PushStyleColor(ImGuiCol_Button, themeManager.AccentColour1);
	else ImGui::PushStyleColor(ImGuiCol_Button, themeManager.SecondaryColour);
	if (ImGui::Button("3D Plane", ImVec2(modeButtonWidth, 40))) { mapDrawViewMode = 2; }
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("(Press L)");
	ImGui::PopStyleColor();
	ImGui::PopStyleVar();
}
void BottomBarDisplay(const ImGuiWindowFlags &window_flags)
{
	ImGui::SetNextWindowPos(ImVec2(0, windowSys.GetWindowRes().y - 25), ImGuiSetCond_Always);
	ImGui::SetNextWindowSize(ImVec2(windowSys.GetWindowRes().x, 25), ImGuiSetCond_Always);
	bool open = true;
	ImGui::Begin("Bottom_Bar", &open, window_flags);
	ImGui::Indent(ImGui::GetContentRegionAvailWidth()*0.5f - 30);
	ImGui::Text(VERSION_NAME.c_str());
	ImGui::End();
}
void SetupImGui()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui_ImplGlfw_InitForOpenGL((GLFWwindow*)windowSys.GetWindow(), true);
	ImGui_ImplOpenGL2_Init();

	ImFont* font = io.Fonts->AddFontFromFileTTF("Resources\\Fonts\\arial.ttf", 16.0f);
	menuBarLargerText = io.Fonts->AddFontFromFileTTF("Resources\\Fonts\\arial.ttf", 18.0f);

	modalWindow.setTitleFont(menuBarLargerText);

	IM_ASSERT(font != NULL);
	IM_ASSERT(menuBarLargerText != NULL);
}
void SetStatesForSavingNormalMap()
{
	glViewport(0, 0, heightMapTexData.getWidth(), heightMapTexData.getHeight());
	fbs.updateTextureDimensions(heightMapTexData.getWidth(), heightMapTexData.getHeight());
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
		mapDrawViewMode = 3;
	if (isKeyPressed(GLFW_KEY_K))
		mapDrawViewMode = 1;
	if (isKeyPressed(GLFW_KEY_L))
		mapDrawViewMode = 2;

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

	if (isKeyPressedDown(GLFW_KEY_Z) && isKeyPressed(GLFW_KEY_LEFT_CONTROL))
	{
		heightMapTexData.updateTextureData(undoRedoSystem.retrieve());
		heightMapTexData.updateTexture();
		if (undoRedoSystem.getCurrentSectionPosition() == 0)
			undoRedoSystem.record(heightMapTexData.getTextureData());
	}

	if (isKeyPressedDown(GLFW_KEY_T))
	{
		if (!windowSys.IsFullscreen())
			windowSys.SetFullscreen(true);
		else
			windowSys.SetFullscreen(false);
	}

	if (isKeyPressedDown(GLFW_KEY_R))
	{
		frameDrawingPanel.getTransform()->setPosition(0, 0);
		zoomLevel = 1;
	}

	if (isKeyPressed(GLFW_KEY_F9))
	{
		windowSys.SetWindowRes(1600, 800);
		isMaximized = false;
	}
}
inline void DisplayBrushSettingsUserInterface(bool &isBlurOn)
{
	ImGui::Text("BRUSH SETTINGS");
	ImGui::Separator();
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 10));
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 5));
	ImGui::Spacing();
	ImGui::PushStyleColor(ImGuiCol_SliderGrab, themeManager.SecondaryColour);
	ImGui::PushStyleColor(ImGuiCol_Button, themeManager.SecondaryColour);

	if (ImGui::Button((isBlurOn) ? "HEIGHT MODE" : "BLUR MODE", ImVec2((int)(ImGui::GetContentRegionAvailWidth() / 2.0f), 40)))
		isBlurOn = !isBlurOn;
	ImGui::SameLine();

	if (isBlurOn)
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);

	if (ImGui::Button((brushData.heightMapPositiveDir) ? "Height -VE" : "Height +VE", ImVec2(ImGui::GetContentRegionAvailWidth(), 40)))
		brushData.heightMapPositiveDir = (isBlurOn) ? brushData.heightMapPositiveDir : !brushData.heightMapPositiveDir;
	if (isBlurOn)
		ImGui::PopStyleVar();
	ImGui::PopStyleColor();
	if (ImGui::SliderFloat(" Brush Scale", &brushData.brushScale, 1.0f, heightMapTexData.getHeight()*0.5f, "%.2f", 1.0f)) {}
	if (ImGui::SliderFloat(" Brush Offset", &brushData.brushOffset, 0.01f, 10.0f, "%.2f", 1.0f)) {}
	if (ImGui::SliderFloat(" Brush Strength", &brushData.brushStrength, 0.0f, 1.0f, "%0.2f", 1.0f)) {}
	if (ImGui::SliderFloat(" Brush Min Height", &brushData.brushMinHeight, 0.0f, 1.0f, "%0.2f", 1.0f)) {}
	if (ImGui::SliderFloat(" Brush Max Height", &brushData.brushMaxHeight, 0.0f, 1.0f, "%0.2f", 1.0f)) {}
	if (ImGui::SliderFloat(" Brush Draw Rate", &brushData.brushRate, 0.0f, heightMapTexData.getHeight() / 2, "%0.2f", 1.0f)) {}
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
	ImGui::PushStyleColor(ImGuiCol_Button, themeManager.SecondaryColour);
	if (ImGui::Button("Flip X-Y", ImVec2(ImGui::GetContentRegionAvailWidth(), 40))) { flipX_Ydir = !flipX_Ydir; }
	ImGui::PopStyleColor();
	const float width = ImGui::GetContentRegionAvailWidth() / 3.0f - 7;

	if (!redChannelActive) ImGui::PushStyleColor(ImGuiCol_Button, themeManager.AccentColour1);
	else ImGui::PushStyleColor(ImGuiCol_Button, themeManager.SecondaryColour);
	if (ImGui::Button("R", ImVec2(width, 40))) { redChannelActive = !redChannelActive; } ImGui::SameLine();
	ImGui::PopStyleColor();

	if (!greenChannelActive) ImGui::PushStyleColor(ImGuiCol_Button, themeManager.AccentColour1);
	else ImGui::PushStyleColor(ImGuiCol_Button, themeManager.SecondaryColour);
	if (ImGui::Button("G", ImVec2(width, 40))) { greenChannelActive = !greenChannelActive; } ImGui::SameLine();
	ImGui::PopStyleColor();

	if (!blueChannelActive) ImGui::PushStyleColor(ImGuiCol_Button, themeManager.AccentColour1);
	else ImGui::PushStyleColor(ImGuiCol_Button, themeManager.SecondaryColour);
	if (ImGui::Button("B", ImVec2(width, 40))) { blueChannelActive = !blueChannelActive; }
	ImGui::PopStyleColor();
}
inline void DisplayLightSettingsUserInterface(float &lightIntensity, float &specularity, float &specularityStrength, glm::vec3 &lightDirection)
{
	ImGui::Text("LIGHT SETTINGS");
	ImGui::Separator();
	if (ImGui::SliderFloat(" Diffuse Intensity", &lightIntensity, 0.0f, 1.0f, "%.2f")) {}
	if (ImGui::SliderFloat(" Specularity", &specularity, 1.0f, 1000.0f, "%.2f")) {}
	if (ImGui::SliderFloat(" Specularity Strength", &specularityStrength, 0.0f, 10.0f, "%.2f")) {}
	ImGui::Text("Light Direction");
	ImGui::PushItemWidth((ImGui::GetContentRegionAvailWidth() / 3.0f) - 7);
	if (ImGui::SliderFloat("## X Angle", &lightDirection.x, 0.01f, 359.99f, "X:%.2f")) {}
	ImGui::SameLine();
	if (ImGui::SliderFloat("## Y Angle", &lightDirection.y, 0.01f, 359.99f, "Y:%.2f")) {}
	ImGui::SameLine();
	if (ImGui::SliderFloat("## Z Angle", &lightDirection.z, 0.01f, 359.99f, "Z:%.2f")) {}
	ImGui::PopItemWidth();
}
inline void DisplayPreview(const ImGuiWindowFlags &window_flags, int &modelViewMode, glm::vec3 &diffuseColour, glm::vec3 &ambientColour, glm::vec3 &lightColour)
{
	bool open = true;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 1.0f);
	ImGui::PushStyleColor(ImGuiCol_SliderGrab, themeManager.SecondaryColour);
	ImGui::SetNextWindowPos(ImVec2(windowSys.GetWindowRes().x - 305, 42), ImGuiSetCond_Always);
	ImGui::SetNextWindowSize(ImVec2(300, windowSys.GetWindowRes().y - 67), ImGuiSetCond_Always);
	ImGui::Begin("Preview_Bar", &open, window_flags);
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
					fileExplorer.displayDialog(FileType::MODEL, [&](std::string str)
					{
						modelPreviewObj = modelLoader.CreateModelFromFile(str);
					});
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
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Load 3d model for preview");

	ImGui::SliderFloat("##Rotation speed", &modelPreviewRotationSpeed, 0, 1, "Rotation Speed:%.2f");
	ImGui::SliderFloat("##Zoom level", &modelPreviewZoomLevel, -1.0f, -100.0f, "Zoom Level:%.2f");
	ImGui::SliderFloat("##Roughness", &modelRoughness, 0.0f, 10.0f, "Roughness:%.2f");
	ImGui::SliderFloat("##Reflectivity", &modelReflectivity, 0.0f, 1.0f, "Reflectivity:%.2f");
	ImGui::PopItemWidth();
	ImGui::Spacing();
	ImGui::Text("VIEW MODE");
	ImGui::Separator();
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 10));
	int modeButtonWidth = (int)(ImGui::GetContentRegionAvailWidth() / 4.0f);
	ImGui::Spacing();

	if (modelViewMode == 3) ImGui::PushStyleColor(ImGuiCol_Button, themeManager.AccentColour1);
	else ImGui::PushStyleColor(ImGuiCol_Button, themeManager.SecondaryColour);
	if (ImGui::Button("Height", ImVec2(modeButtonWidth - 5, 40))) { modelViewMode = 3; }
	ImGui::PopStyleColor();

	ImGui::SameLine(0, 5);
	if (modelViewMode == 1) ImGui::PushStyleColor(ImGuiCol_Button, themeManager.AccentColour1);
	else ImGui::PushStyleColor(ImGuiCol_Button, themeManager.SecondaryColour);
	if (ImGui::Button("Normal", ImVec2(modeButtonWidth - 5, 40))) { modelViewMode = 1; }
	ImGui::PopStyleColor();

	ImGui::SameLine(0, 5);
	if (modelViewMode == 2) ImGui::PushStyleColor(ImGuiCol_Button, themeManager.AccentColour1);
	else ImGui::PushStyleColor(ImGuiCol_Button, themeManager.SecondaryColour);
	if (ImGui::Button("Lighting", ImVec2(modeButtonWidth, 40))) { modelViewMode = 2; }
	ImGui::PopStyleColor();

	ImGui::SameLine(0, 5);
	if (modelViewMode == 4) ImGui::PushStyleColor(ImGuiCol_Button, themeManager.AccentColour1);
	else ImGui::PushStyleColor(ImGuiCol_Button, themeManager.SecondaryColour);
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
		static char diffuseTextureImageLocation[500] = "Resources\\Textures\\crate.jpg";
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 5));
		ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth()*0.7f);
		ImGui::InputText("## Preview Image Location", diffuseTextureImageLocation, sizeof(diffuseTextureImageLocation));
		ImGui::PopItemWidth();
		ImGui::SameLine(0, 5);
		if (ImGui::Button("LOAD", ImVec2(ImGui::GetContentRegionAvailWidth() + 5, 27)))
		{
			currentLoadingOption = LoadingOption::TEXTURE;
			fileExplorer.displayDialog(FileType::IMAGE, [&](std::string str)
			{
				for (unsigned int i = 0; i < str.length(); i++)
					diffuseTextureImageLocation[i] = str[i];
				if (currentLoadingOption == LoadingOption::TEXTURE)
				{
					diffuseTexDataForPreview.SetTexId(TextureManager::loadTextureFromFile(str, false));
				}
			});
		}
		ImGui::PopStyleVar();
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Load texture for preview model");
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
		ImGui::PushFont(menuBarLargerText);
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open Project", "CTRL+O"))
			{
			}
			if (ImGui::MenuItem("Open Scene")) {}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit"))
		{
			bool isUndoDisabled = undoRedoSystem.getCurrentSectionPosition() == 1 ? true : false;
			bool isRedoDisabled = undoRedoSystem.getCurrentSectionPosition() >= 1 && undoRedoSystem.getCurrentSectionPosition() <= undoRedoSystem.getMaxUndoSteps() - 1 ? false : true;

			if (isUndoDisabled)
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
			if (ImGui::MenuItem("Undo", "CTRL+Z"))
			{
				heightMapTexData.updateTextureData(undoRedoSystem.retrieve());
				heightMapTexData.updateTexture();
			}
			if (isUndoDisabled)
				ImGui::PopStyleVar();
			if (isRedoDisabled)
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
			if (ImGui::MenuItem("Redo", "CTRL+Y"))
			{
			}
			if (isRedoDisabled)
				ImGui::PopStyleVar();
			ImGui::EndMenu();
		}

		const char* items[] = { "    Default Theme", "    Dark Theme", "    Light Theme", "    Blue Theme" };
		static int item_current = 0;
		ImGui::PushItemWidth(180);
		ImGui::Combo("##combo", &item_current, items, IM_ARRAYSIZE(items));
		ImGui::PopItemWidth();
		ImGui::PopFont();
		switch (item_current)
		{
		case 0:
			themeManager.EnableInBuiltTheme(ThemeManager::Theme::DEFAULT);
			break;
		case 1:
			themeManager.EnableInBuiltTheme(ThemeManager::Theme::DARK);
			break;
		case 2:
			themeManager.EnableInBuiltTheme(ThemeManager::Theme::LIGHT);
			break;
		case 3:
			themeManager.EnableInBuiltTheme(ThemeManager::Theme::BLUE);
			break;
		default:
			break;
		}
		ImGui::PopStyleVar();
#ifdef NORA_CUSTOM_WINDOW_CHROME
		ImGui::Indent(windowSys.GetWindowRes().x - 160);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(5, 10));
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		if (isUsingCustomTheme)
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ACCENT_COL);
		if (ImGui::ImageButton((ImTextureID)minimizeTexture, ImVec2(30, 30), ImVec2(0, 0), ImVec2(1, 1), 5)) { windowSys.Minimize(); }
		if (ImGui::ImageButton((ImTextureID)restoreTexture, ImVec2(30, 30), ImVec2(0, 0), ImVec2(1, 1), 5))
		{
			if (windowSys.IsFullscreen())
				windowSys.SetFullscreen(false);
			else
				windowSys.SetFullscreen(true);
		}
		if (ImGui::ImageButton((ImTextureID)closeTexture, ImVec2(30, 30), ImVec2(0, 0), ImVec2(1, 1), 5)) { windowSys.Close(); }
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

		const int nSize = heightMapTexData.getWidth() * heightMapTexData.getHeight() * 3;
		char* dataBuffer = new char[nSize];
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, dataBuffer);

		glBindTexture(GL_TEXTURE_2D, 0);
		fbs.updateTextureDimensions(windowSys.GetWindowRes().x, windowSys.GetWindowRes().y);
		stbi_write_bmp(locationStr.c_str(), heightMapTexData.getWidth(), heightMapTexData.getHeight(), 3, dataBuffer);
		delete dataBuffer;
	}
}
inline void HandleLeftMouseButtonInput_NormalMapInteraction(int state, glm::vec2 &prevMouseCoord, DrawingPanel &frameDrawingPanel, bool isBlurOn)
{
	static int prevState = GLFW_RELEASE;
	static bool didActuallyDraw = false;
	const glm::vec2 INVALID = glm::vec2(-100000000, -10000000);

	if (state == GLFW_PRESS)
	{
		glm::vec2 currentMouseCoord = windowSys.GetCursorPos();
		glm::vec2 wnCurMouse = glm::vec2(currentMouseCoord.x / windowSys.GetWindowRes().x, 1.0f - currentMouseCoord.y / windowSys.GetWindowRes().y);
		glm::vec2 wnPrevMouse = glm::vec2(prevMouseCoord.x / windowSys.GetWindowRes().x, 1.0f - prevMouseCoord.y / windowSys.GetWindowRes().y);
		//viewport current mouse coords
		glm::vec2 vpCurMouse(wnCurMouse.x * 2.0f - 1.0f, wnCurMouse.y * 2.0f - 1.0f);
		//viewport previous mouse coords
		glm::vec2 vpPrevMouse(wnPrevMouse.x * 2.0f - 1.0f, wnPrevMouse.y * 2.0f - 1.0f);

		float minDistThresholdForDraw = brushData.brushRate;
		float distOfPrevAndCurrentMouseCoord = glm::distance(wnCurMouse, wnPrevMouse);

		if (currentMouseCoord != prevMouseCoord)
		{
			glm::vec2 midPointWorldPos = frameDrawingPanel.getTransform()->getPosition();
			glm::vec2 topRightCorner;
			glm::vec2 bottomLeftCorner;
			topRightCorner.x = midPointWorldPos.x + frameDrawingPanel.getTransform()->getScale().x;
			topRightCorner.y = midPointWorldPos.y + frameDrawingPanel.getTransform()->getScale().y;
			bottomLeftCorner.x = midPointWorldPos.x - frameDrawingPanel.getTransform()->getScale().x;
			bottomLeftCorner.y = midPointWorldPos.y - frameDrawingPanel.getTransform()->getScale().y;

			if (vpCurMouse.x > bottomLeftCorner.x && vpCurMouse.x < topRightCorner.x &&
				vpCurMouse.y > bottomLeftCorner.y && vpCurMouse.y < topRightCorner.y &&
				distOfPrevAndCurrentMouseCoord > minDistThresholdForDraw)
			{
				float curX = (vpCurMouse.x - bottomLeftCorner.x) / glm::abs((topRightCorner.x - bottomLeftCorner.x));
				float curY = (vpCurMouse.y - bottomLeftCorner.y) / glm::abs((topRightCorner.y - bottomLeftCorner.y));

				const float maxWidth = heightMapTexData.getWidth();
				const float convertedBrushScale = (brushData.brushScale / heightMapTexData.getHeight()) * maxWidth * 3.5f;

				if (!isBlurOn)
				{
					float density = 0.01f;
					if (distOfPrevAndCurrentMouseCoord > density)
					{
						float prevX = (vpPrevMouse.x - bottomLeftCorner.x) / glm::abs((topRightCorner.x - bottomLeftCorner.x));
						float prevY = (vpPrevMouse.y - bottomLeftCorner.y) / glm::abs((topRightCorner.y - bottomLeftCorner.y));

						glm::vec2 prevPoint(prevX, prevY);
						glm::vec2 toPoint(curX, curY);
						glm::vec2 iterCurPoint = prevPoint;

						float density = 0.01f;
						glm::vec2 diff = (prevPoint - toPoint);
						glm::vec2 incValue = glm::normalize(diff) * density;
						int numberOfPoints = glm::floor(glm::clamp(glm::length(diff) / density, 1.0f, 10.0f));

						for (int i = 0; i < numberOfPoints; i++)
						{
							float left = glm::clamp(iterCurPoint.x * maxWidth - convertedBrushScale, 0.0f, maxWidth);
							float right = glm::clamp(iterCurPoint.x * maxWidth + convertedBrushScale, 0.0f, maxWidth);
							float bottom = glm::clamp(iterCurPoint.y * maxWidth - convertedBrushScale, 0.0f, maxWidth);
							float top = glm::clamp(iterCurPoint.y * maxWidth + convertedBrushScale, 0.0f, maxWidth);
							iterCurPoint += incValue;
							SetPixelValues(heightMapTexData, left, right, bottom, top, iterCurPoint.x, iterCurPoint.y);
						}
					}
					else
					{
						float left = glm::clamp((curX - convertedBrushScale) * maxWidth, 0.0f, maxWidth);
						float right = glm::clamp((curX + convertedBrushScale) * maxWidth, 0.0f, maxWidth);
						float bottom = glm::clamp((curY - convertedBrushScale) * maxWidth, 0.0f, maxWidth);
						float top = glm::clamp((curY + convertedBrushScale) * maxWidth, 0.0f, maxWidth);
						SetPixelValues(heightMapTexData, left, right, bottom, top, curX, curY);
					}
				}
				else if (isBlurOn)
				{
					float left = glm::clamp((curX - convertedBrushScale) * maxWidth, 0.0f, maxWidth);
					float right = glm::clamp((curX + convertedBrushScale) * maxWidth, 0.0f, maxWidth);
					float bottom = glm::clamp((curY - convertedBrushScale) * maxWidth, 0.0f, maxWidth);
					float top = glm::clamp((curY + convertedBrushScale) * maxWidth, 0.0f, maxWidth);
					SetBluredPixelValues(heightMapTexData, left, right, bottom, top, curX, curY);
				}
				didActuallyDraw = true;
				heightMapTexData.setTextureDirty();
				prevMouseCoord = currentMouseCoord;
			}
			else //Is not within the panel bounds
			{
				didActuallyDraw = false;
			}
		}//Check if same mouse position
	}
	else //Not pressing left-mouse button
	{
		if (prevState == GLFW_PRESS && didActuallyDraw)
		{
			undoRedoSystem.record(heightMapTexData.getTextureData());
			didActuallyDraw = false;
		}
		prevMouseCoord = glm::vec2(-10, -10);
	}
	prevState = state;
}

inline void HandleLeftMouseButtonInput_UI(int state, glm::vec2 &initPos, WindowSide &windowSideAtInitPos, double x, double y, bool &isMaximized, glm::vec2 &prevGlobalFirstMouseCoord)
{
#if NORA_CUSTOM_WINDOW_CHROME
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
#endif
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

inline void SetPixelValues(TextureData& inputTexData, int startX, int endX, int startY, int endY, double xpos, double ypos)
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
			rVal = colData.getColour_32_Bit().r;
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

inline void SetBluredPixelValues(TextureData& inputTexData, int startX, int endX, int startY, int endY, double xpos, double ypos)
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
				float avg = tempPixelData[index].getColour_32_Bit().r * 0.5f;

				int leftIndex = ((i - 1) - startX)*(endX - startX) + (j - startY);
				int rightIndex = ((i + 1) - startX)*(endX - startX) + (j - startY);
				int topIndex = (i - startX)*(endX - startX) + ((j + 1) - startY);
				int bottomIndex = (i - startX)*(endX - startX) + ((j - 1) - startY);

				int topLeftIndex = ((i - 1) - startX)*(endX - startX) + ((j + 1) - startY);
				int bottomLeftIndex = ((i - 1) - startX)*(endX - startX) + ((j - 1) - startY);
				int topRightIndex = ((i + 1) - startX)*(endX - startX) + ((j + 1) - startY);
				int bottomRightIndex = ((i + 1) - startX)*(endX - startX) + ((j - 1) - startY);

				int kernel[] = { leftIndex, rightIndex, topIndex, bottomIndex, topLeftIndex, bottomLeftIndex, topRightIndex, bottomRightIndex };
				//not clamping values based in width and heifhgt of current pixel center
				for (unsigned int i = 0; i < 8; i++)
					avg += (kernel[i] >= 0 && kernel[i] < totalPixelCount) ? tempPixelData[kernel[i]].getColour_32_Bit().r * 0.0625f : 0.01f;
				float pixelCol = tempPixelData[index].getColour_32_Bit().r;
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
bool isKeyPressedDown(int key)
{
	static int prevKey = GLFW_KEY_0;
	int state = glfwGetKey((GLFWwindow*)windowSys.GetWindow(), key);

	if (state == GLFW_PRESS)
	{
		if (prevKey != key)
		{
			prevKey = key;
			return true;
		}
		prevKey = key;
	}
	else if (state == GLFW_RELEASE)
	{
		if (prevKey == key)
			prevKey = GLFW_KEY_0;
	}
	return false;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	width = glm::clamp(width, WINDOW_SIZE_MIN, (int)windowSys.GetMaxWindowRes().x);
	height = glm::clamp(height, WINDOW_SIZE_MIN, (int)windowSys.GetMaxWindowRes().y);

	windowSys.SetWindowRes(width, height);
	fbs.updateTextureDimensions(windowSys.GetWindowRes());
	previewFbs.updateTextureDimensions(windowSys.GetWindowRes());
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) noexcept
{
	zoomLevel += zoomLevel * 0.1f * yoffset;
}