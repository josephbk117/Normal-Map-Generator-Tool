#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <queue>
#include <GL\glew.h>
#include <GLFW/glfw3.h>
#include <GLM\gtc\quaternion.hpp>
#include <GLM\gtx\quaternion.hpp>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl2.h"
#include "stb_image_write.h"

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
#include "ViewBasedUtilities.h"
#include "UndoRedoSystem.h"
#include "MeshLoadingSystem.h"

//TODO : * Done but not good enough *Implement mouse position record and draw to prevent cursor skipping ( probably need separate thread for drawing |completly async| )
//TODO : Add Uniform Buffers
//TODO : Add shadows and an optional plane
//TODO : Mouse control when preview maximize panel opens
//TODO : Add layers, Definition for layer type can be height map / direct normal map. | Use various blending methods |
//TODO : Look into converting normal map to heightmap for editing purposes
//TODO : Control directional light direction through 3D hemisphere sun object in preview screen
//TODO : Add preferences tab : max undo slots, max image size(requires app restart), export image size [ any resolution / percentage of current resolution ], default theme
//TODO : Reset view should make non 1:1 images fit in screen
//TODO : Convert text to icon for most buttons
//TODO : Add texture slots for [ Diffuse & Specular ] in preview in Textured mode
//TODO : Fix memory error while using custom brush texture and exit the application
//TODO : Parallax map option
//TODO : Add PBR shader workflow support
//TODO : Custom shader support for preview
//TODO : Better lighting options
//TODO : Moving the panel anywhere in the window and not zoom level effcted
//TODO : radio button for height mode /blur mode

//#define NORA_CUSTOM_WINDOW_CHROME //For custom window chrome

enum class LoadingOption
{
	MODEL, TEXTURE, NONE
};

const std::string VERSION_NAME = "v1.0 Beta";
const std::string FONTS_PATH = "Resources\\Fonts\\";
const std::string THEMES_PATH = "Resources\\Themes\\";
const std::string TEXTURES_PATH = "Resources\\Textures\\";
const std::string CUBEMAP_TEXTURES_PATH = "Resources\\Cubemap Textures\\";
const std::string BRUSH_TEXTURES_PATH = "Resources\\Brushes\\";
const std::string UI_TEXTURES_PATH = "Resources\\UI\\";
const std::string SHADERS_PATH = "Resources\\Shaders\\";
const std::string MODELS_PATH = "Resources\\3D Models\\Primitives\\";
const std::string CUBE_MODEL_PATH = MODELS_PATH + "Cube.obj";
const std::string CYLINDER_MODEL_PATH = MODELS_PATH + "Cylinder.obj";
const std::string SPHERE_MODEL_PATH = MODELS_PATH + "Sphere.obj";
const std::string TORUS_MODEL_PATH = MODELS_PATH + "Torus.obj";
const std::string PLANE_MODEL_PATH = MODELS_PATH + "Plane.obj";

const int WINDOW_SIZE_MIN = 480;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) noexcept;
void SetPixelValues(TextureData& texData, int startX, int width, int startY, int height, double xpos, double ypos);
void SetPixelValuesWithBrushTexture(TextureData& inputTexData, TextureData& brushTexture, int startX, int endX, int startY, int endY, double xpos, double ypos);
void SetBluredPixelValues(TextureData& inputTexData, int startX, int width, int startY, int height, double xpos, double ypos);
void SaveNormalMapToFile(const std::string &locationStr, ImageFormat imageFormat);
inline void HandleMiddleMouseButtonInput(int state, glm::vec2 &prevMiddleMouseButtonCoord, double deltaTime, DrawingPanel &normalmapPanel);
inline void HandleLeftMouseButtonInput_UI(int state, glm::vec2 &initPos, WindowSide &windowSideAtInitPos, double x, double y, bool &isMaximized, glm::vec2 &prevGlobalFirstMouseCoord);
inline void HandleLeftMouseButtonInput_NormalMapInteraction(int state, DrawingPanel &normalmapPanel, bool isBlurOn);
inline void DisplayWindowTopBar(unsigned int minimizeTexture, unsigned int restoreTexture, bool &isMaximized, unsigned int closeTexture);
inline void DisplayBottomBar(const ImGuiWindowFlags &window_flags);
inline void DisplaySideBar(const ImGuiWindowFlags &window_flags, DrawingPanel &frameDrawingPanel, char saveLocation[500], bool &shouldSaveNormalMap, bool &changeSize);
inline void DisplayPreview(const ImGuiWindowFlags &window_flags);
inline void DisplayLightSettingsUserInterface();
inline void DisplayNormalSettingsUserInterface();
inline void DisplayBrushSettingsUserInterface(bool &isBlurOn);
inline void HandleKeyboardInput(double deltaTime, DrawingPanel &frameDrawingPanel, bool &isMaximized);
void SetStatesForSavingNormalMap();
void SetupImGui();

MeshLoadingSystem::MeshLoader modelLoader;
ImFont* menuBarLargerText = NULL;
PreviewStateUtility previewStateUtility;
NormalViewStateUtility normalViewStateUtility;

FrameBufferSystem fbs;
FrameBufferSystem previewFbs;

BrushData brushData;
TextureData heightMapTexData;
TextureData diffuseTexDataForPreview;
TextureData specularTexDataForPreview;
ModelObject *modelPreviewObj = nullptr;
LoadingOption currentLoadingOption = LoadingOption::NONE;
FileExplorer fileExplorer;
ModalWindow modalWindow;
WindowSystem windowSys;
ThemeManager themeManager;
DrawingPanel normalmapPanel;
UndoRedoSystem undoRedoSystem(512 * 512 * 4 * 20, 512 * 512 * 4);
WindowSide windowSideVal;

unsigned int toggleFullscreenTexId, resetViewTexId, clearViewTexId, maximizePreviewTexId;
char **themeItems = nullptr;

struct BoundsAndPos
{
public:
	glm::vec2 mouseCoord;
	float left, right, bottom, up;
};

std::queue<BoundsAndPos> mouseCoordQueue;


void ApplyChangesToPanel()
{
	const float maxWidth = heightMapTexData.getRes().x;
	const float convertedBrushScale = (brushData.brushScale / heightMapTexData.getRes().y) * maxWidth * 3.5f;

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
	glEnable(GL_BLEND);

	SetupImGui();
	themeManager.Init();
	themeManager.EnableInBuiltTheme(ThemeManager::Theme::DEFAULT);

	windowSys.SetFrameBufferResizeCallback(framebuffer_size_callback);
	windowSys.SetScrollCallback(scroll_callback);
	modelPreviewObj = modelLoader.CreateModelFromFile(CUBE_MODEL_PATH); // Default loaded model in preview window

	ModelObject* cubeForSkybox = modelLoader.CreateModelFromFile(CUBE_MODEL_PATH);
	ModelObject* previewPlane = modelLoader.CreateModelFromFile(PLANE_MODEL_PATH);

	normalmapPanel.init(1.0f, 1.0f);
	DrawingPanel frameDrawingPanel;
	frameDrawingPanel.init(1.0f, 1.0f);
	DrawingPanel previewFrameDrawingPanel;
	previewFrameDrawingPanel.init(1.0f, 1.0f);
	DrawingPanel brushPanel;
	brushPanel.init(1.0f, 1.0f);

	//Windowing related images
	unsigned int closeTextureId = TextureManager::loadTextureFromFile(UI_TEXTURES_PATH + "closeIcon.png");
	unsigned int restoreTextureId = TextureManager::loadTextureFromFile(UI_TEXTURES_PATH + "maxWinIcon.png");
	unsigned int minimizeTextureId = TextureManager::loadTextureFromFile(UI_TEXTURES_PATH + "toTrayIcon.png");
	unsigned int logoTextureId = TextureManager::loadTextureFromFile(UI_TEXTURES_PATH + "icon.png");

	toggleFullscreenTexId = TextureManager::loadTextureFromFile(UI_TEXTURES_PATH + "toggleFullscreen.png");
	clearViewTexId = TextureManager::loadTextureFromFile(UI_TEXTURES_PATH + "clearView.png");
	resetViewTexId = TextureManager::loadTextureFromFile(UI_TEXTURES_PATH + "resetLocation.png");
	maximizePreviewTexId = TextureManager::loadTextureFromFile(UI_TEXTURES_PATH + "maximizePreview.png");

	std::vector<std::string> cubeMapImagePaths;
	cubeMapImagePaths.push_back(CUBEMAP_TEXTURES_PATH + "Sahara Desert Cubemap\\sahara_lf.tga");
	cubeMapImagePaths.push_back(CUBEMAP_TEXTURES_PATH + "Sahara Desert Cubemap\\sahara_rt.tga");
	cubeMapImagePaths.push_back(CUBEMAP_TEXTURES_PATH + "Sahara Desert Cubemap\\sahara_dn.tga");
	cubeMapImagePaths.push_back(CUBEMAP_TEXTURES_PATH + "Sahara Desert Cubemap\\sahara_up.tga");
	cubeMapImagePaths.push_back(CUBEMAP_TEXTURES_PATH + "Sahara Desert Cubemap\\sahara_ft.tga");
	cubeMapImagePaths.push_back(CUBEMAP_TEXTURES_PATH + "Sahara Desert Cubemap\\sahara_bk.tga");

	unsigned int cubeMapTextureId = TextureManager::loadCubemapFromFile(cubeMapImagePaths);
	diffuseTexDataForPreview.SetTexId(TextureManager::loadTextureFromFile(TEXTURES_PATH + "Wall Tex Diffuse.png"));
	specularTexDataForPreview.SetTexId(TextureManager::loadTextureFromFile(TEXTURES_PATH + "Wall Tex Specular.png"));

	TextureManager::getTextureDataFromFile(TEXTURES_PATH + "Wall Tex Height.png", heightMapTexData);
	heightMapTexData.SetTexId(TextureManager::loadTextureFromData(heightMapTexData));
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

	ShaderProgram gridLineShader;
	gridLineShader.compileShaders(SHADERS_PATH + "gridLines.vs", SHADERS_PATH + "gridLines.fs");
	gridLineShader.linkShaders();

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
	int methodIndexUniform = normalmapShader.getUniformLocation("_MethodIndex");

	int brushPreviewModelUniform = brushPreviewShader.getUniformLocation("model");
	int brushPreviewTextureUniform = brushPreviewShader.getUniformLocation("_BrushTexture");
	int brushPreviewOffsetUniform = brushPreviewShader.getUniformLocation("_BrushOffset");
	int brushPreviewStrengthUniform = brushPreviewShader.getUniformLocation("_BrushStrength");
	int brushPreviewColourUniform = brushPreviewShader.getUniformLocation("_BrushColour");
	int brushPreviewUseTextureUniform = brushPreviewShader.getUniformLocation("_UseTexture");

	int modelPreviewModelUniform = modelViewShader.getUniformLocation("model");
	int modelPreviewViewUniform = modelViewShader.getUniformLocation("view");
	int modelPreviewProjectionUniform = modelViewShader.getUniformLocation("projection");
	int modelCameraPos = modelViewShader.getUniformLocation("_CameraPosition");
	int modelWidthUniform = modelViewShader.getUniformLocation("_HeightmapDimX");
	int modelHeightUniform = modelViewShader.getUniformLocation("_HeightmapDimY");
	int modelNormalMapModeUniform = modelViewShader.getUniformLocation("_normalMapModeOn");
	int modelNormalMapStrengthUniform = modelViewShader.getUniformLocation("_HeightmapStrength");
	int modelLightIntensityUniform = modelViewShader.getUniformLocation("_LightIntensity");
	int modelLightSpecularityUniform = modelViewShader.getUniformLocation("_Specularity");
	int modelLightSpecularityStrengthUniform = modelViewShader.getUniformLocation("_SpecularStrength");
	int modelRoughnessUniform = modelViewShader.getUniformLocation("_Roughness");
	int modelReflectivityUniform = modelViewShader.getUniformLocation("_Reflectivity");
	int modelLightPositionUniform = modelViewShader.getUniformLocation("lightPos");
	int modelLightColourUniform = modelViewShader.getUniformLocation("lightColour");
	int modelDiffuseColourUniform = modelViewShader.getUniformLocation("diffuseColour");
	int modelHeightMapTextureUniform = modelViewShader.getUniformLocation("inTexture");
	int modelTextureMapTextureUniform = modelViewShader.getUniformLocation("inTexture2");
	int modelSpecularMapTextureUniform = modelViewShader.getUniformLocation("inTexture3");
	int modelCubeMapTextureUniform = modelViewShader.getUniformLocation("skybox");
	int modelMethodIndexUniform = modelViewShader.getUniformLocation("_MethodIndex");

	int gridLineModelMatrixUniform = gridLineShader.getUniformLocation("model");
	int gridLineViewMatrixUniform = gridLineShader.getUniformLocation("view");
	int gridLineProjectionMatrixUniform = gridLineShader.getUniformLocation("projection");

	bool isMaximized = false;
	bool isBlurOn = false;

	brushData.brushScale = 25.0f;
	brushData.brushOffset = 0.25f;
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
	glm::vec2 prevWindowSize = glm::vec2(500, 500);

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

		const glm::vec2 curMouseCoord = windowSys.GetCursorPos();
		HandleKeyboardInput(deltaTime, frameDrawingPanel, isMaximized);

		fbs.BindFrameBuffer();
		glClearColor(0.9f, 0.5f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		glBindTexture(GL_TEXTURE_2D, heightMapTexData.GetTexId());
		normalmapShader.use();

		const WindowSide currentMouseCoordWindowSide = WindowTransformUtility::GetWindowSideBorderAtMouseCoord(curMouseCoord, windowSys.GetWindowRes());
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
		{
			if (heightMapTexData.getRes().x > heightMapTexData.getRes().y)
				aspectRatioHolder = glm::vec2(1, aspectRatio) * glm::vec2(heightMapTexData.getRes().x / heightMapTexData.getRes().y, 1);
			else
				aspectRatioHolder = glm::vec2(1, aspectRatio) * glm::vec2(1, heightMapTexData.getRes().y / heightMapTexData.getRes().x);
		}
		else
		{
			if (heightMapTexData.getRes().x > heightMapTexData.getRes().y)
				aspectRatioHolder = glm::vec2(1.0f / aspectRatio, 1) * glm::vec2(heightMapTexData.getRes().x / heightMapTexData.getRes().y, 1);
			else
				aspectRatioHolder = glm::vec2(1.0f / aspectRatio, 1) * glm::vec2(1, heightMapTexData.getRes().y / heightMapTexData.getRes().x);
		}
		frameDrawingPanel.getTransform()->setScale(aspectRatioHolder * normalViewStateUtility.zoomLevel);

		float leftBorder = 300.0f / windowSys.GetWindowRes().x;
		float rightBorder = 1.0f - leftBorder;

		float topBorder = 1.0f / windowSys.GetWindowRes().y;
		float bottomBorder = 1.0f - topBorder;

		leftBorder = (leftBorder * 2.0f - 1.0f);
		rightBorder = (rightBorder * 2.0f - 1.0f);
		topBorder = (topBorder * 2.0f - 1.0f)*1.25f;
		bottomBorder = (bottomBorder * 2.0f - 1.0f)*1.25f;

		//frameDrawingPanel.getTransform()->setX(glm::clamp(frameDrawingPanel.getTransform()->getPosition().x, leftBorder, rightBorder));
		//frameDrawingPanel.getTransform()->setY(glm::clamp(frameDrawingPanel.getTransform()->getPosition().y, topBorder, bottomBorder));
		frameDrawingPanel.getTransform()->update();

		const int leftMouseButtonState = glfwGetMouseButton((GLFWwindow*)windowSys.GetWindow(), GLFW_MOUSE_BUTTON_LEFT);
		const int middleMouseButtonState = glfwGetMouseButton((GLFWwindow*)windowSys.GetWindow(), GLFW_MOUSE_BUTTON_MIDDLE);

		//Have to set various bounds on mouse location to determine context driven mouse actions

		windowSideVal = WindowTransformUtility::GetWindowAreaAtMouseCoord(curMouseCoord.x, curMouseCoord.y, windowSys.GetWindowRes().x, windowSys.GetWindowRes().y);
		if (windowSideVal == WindowSide::CENTER)
		{
			HandleMiddleMouseButtonInput(middleMouseButtonState, prevMiddleMouseButtonCoord, deltaTime, frameDrawingPanel);
			HandleLeftMouseButtonInput_NormalMapInteraction(leftMouseButtonState, frameDrawingPanel, isBlurOn);
		}
		HandleLeftMouseButtonInput_UI(leftMouseButtonState, initPos, windowSideAtInitPos, curMouseCoord.x, curMouseCoord.y, isMaximized, prevGlobalFirstMouseCoord);
		heightMapTexData.updateTexture();

		normalmapPanel.getTransform()->update();
		//---- Applying Normal Map Shader Uniforms---//
		normalmapShader.applyShaderUniformMatrix(normalPanelModelMatrixUniform, normalmapPanel.getTransform()->getMatrix());
		normalmapShader.applyShaderFloat(strengthValueUniform, normalViewStateUtility.normalMapStrength);
		normalmapShader.applyShaderFloat(specularityUniform, normalViewStateUtility.specularity);
		normalmapShader.applyShaderFloat(specularityStrengthUniform, normalViewStateUtility.specularityStrength);
		normalmapShader.applyShaderFloat(lightIntensityUniform, normalViewStateUtility.lightIntensity);
		normalmapShader.applyShaderVector3(lightDirectionUniform, normalViewStateUtility.getNormalizedLightDir());
		normalmapShader.applyShaderFloat(widthUniform, heightMapTexData.getRes().x);
		normalmapShader.applyShaderFloat(heightUniform, heightMapTexData.getRes().y);
		normalmapShader.applyShaderInt(normalMapModeOnUniform, normalViewStateUtility.mapDrawViewMode);
		normalmapShader.applyShaderBool(flipXYdirUniform, normalViewStateUtility.flipX_Ydir);
		normalmapShader.applyShaderBool(RedChannelUniform, normalViewStateUtility.redChannelActive);
		normalmapShader.applyShaderBool(GreenChannelUniform, normalViewStateUtility.greenChannelActive);
		normalmapShader.applyShaderBool(BlueChannelUniform, normalViewStateUtility.blueChannelActive);
		normalmapShader.applyShaderBool(methodIndexUniform, normalViewStateUtility.methodIndex);
		normalmapPanel.draw();

		static char saveLocation[500] = "C:\\NoraNormalMap.tga";
		if (shouldSaveNormalMap)
		{
			ImageFormat imageFormat;
			//Image validation stage start
			std::string path(saveLocation);
			std::string fileExt = fileExplorer.getFileExtension(saveLocation);
			if (fileExt == ".tga")
				imageFormat = ImageFormat::TGA;
			else if (fileExt == ".bmp")
				imageFormat = ImageFormat::BMP;
			else if (fileExt == ".png")
				imageFormat = ImageFormat::PNG;
			else if (fileExt == ".jpg")
				imageFormat = ImageFormat::JPEG;
			else //Un-supported format
			{
				shouldSaveNormalMap = false;
				if (fileExt == "")
					modalWindow.setModalDialog("ERROR", "The provided path : " + path + "\nIs incomplete, Check if the path is valid");
				else
					modalWindow.setModalDialog("ERROR", "The extension '" + fileExt + "' is not supported\n Choose from .png, .jpg, .tga or .bmp");
				fbs.updateTextureDimensions(windowSys.GetWindowRes().x, windowSys.GetWindowRes().y);
				continue;
			}
			//Image validation stage over
			SaveNormalMapToFile(saveLocation, imageFormat);
			shouldSaveNormalMap = false;
			modalWindow.setModalDialog("INFO", "Image exported to : " + path + "\nResolution : " +
				std::to_string((int)heightMapTexData.getRes().x) + "x" + std::to_string((int)heightMapTexData.getRes().y));
			continue;
		}

		if (windowSys.IsKeyPressedDown(GLFW_KEY_F10))
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
		frameDrawingPanel.setTextureID(fbs.getColourTexture());
		frameDrawingPanel.draw();

		previewFbs.BindFrameBuffer();

		glEnable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		static float circleAround = 0.5f;
		static float yAxis = -2.0f;
		glm::vec3 cameraPosition;
		static glm::vec2 prevMcord;
		glm::vec2 offset = (prevMcord - windowSys.GetCursorPos());
		if (leftMouseButtonState == GLFW_PRESS && glm::length(offset) > 0.0f && windowSideVal == WindowSide::RIGHT && curMouseCoord.y < 400)
		{
			circleAround += offset.x*0.01f;
			yAxis += offset.y*0.01f;
		}
		yAxis = glm::clamp(yAxis, -100.0f, 100.0f);
		cameraPosition.x = glm::sin(circleAround) * previewStateUtility.modelPreviewZoomLevel;
		cameraPosition.z = glm::cos(circleAround) * previewStateUtility.modelPreviewZoomLevel;
		cameraPosition.y = yAxis;

		if (glm::distance(cameraPosition, glm::vec3(0)) > previewStateUtility.modelPreviewZoomLevel)
		{
			cameraPosition = -glm::normalize(cameraPosition) * previewStateUtility.modelPreviewZoomLevel;
		}
		prevMcord = windowSys.GetCursorPos();

		modelViewShader.use();
		modelViewShader.applyShaderUniformMatrix(modelPreviewModelUniform, glm::mat4());
		modelViewShader.applyShaderUniformMatrix(modelPreviewViewUniform, glm::lookAt(cameraPosition, glm::vec3(0), glm::vec3(0, 1, 0)));
		modelViewShader.applyShaderUniformMatrix(modelPreviewProjectionUniform, glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f));
		modelViewShader.applyShaderVector3(modelCameraPos, cameraPosition);
		modelViewShader.applyShaderInt(modelNormalMapModeUniform, previewStateUtility.modelViewMode);
		modelViewShader.applyShaderFloat(modelNormalMapStrengthUniform, normalViewStateUtility.normalMapStrength);
		modelViewShader.applyShaderFloat(modelWidthUniform, heightMapTexData.getRes().x);
		modelViewShader.applyShaderFloat(modelHeightUniform, heightMapTexData.getRes().y);
		modelViewShader.applyShaderFloat(modelLightIntensityUniform, previewStateUtility.lightIntensity);
		modelViewShader.applyShaderFloat(modelLightSpecularityUniform, normalViewStateUtility.specularity);
		modelViewShader.applyShaderFloat(modelLightSpecularityStrengthUniform, normalViewStateUtility.specularityStrength);
		modelViewShader.applyShaderFloat(modelRoughnessUniform, previewStateUtility.modelRoughness);
		modelViewShader.applyShaderFloat(modelReflectivityUniform, previewStateUtility.modelReflectivity);
		glm::vec3 lightPos;
		lightPos.x = sin(previewStateUtility.lightLocation) * 3;
		lightPos.z = cos(previewStateUtility.lightLocation) * 3;
		modelViewShader.applyShaderVector3(modelLightPositionUniform, lightPos);
		modelViewShader.applyShaderInt(modelHeightMapTextureUniform, 0);
		modelViewShader.applyShaderInt(modelTextureMapTextureUniform, 1);
		modelViewShader.applyShaderInt(modelSpecularMapTextureUniform, 2);
		modelViewShader.applyShaderInt(modelCubeMapTextureUniform, 3);

		modelViewShader.applyShaderVector3(modelDiffuseColourUniform, previewStateUtility.diffuseColour);
		modelViewShader.applyShaderVector3(modelLightColourUniform, previewStateUtility.lightColour);
		modelViewShader.applyShaderBool(modelMethodIndexUniform, normalViewStateUtility.methodIndex);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, heightMapTexData.GetTexId());
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, diffuseTexDataForPreview.GetTexId());
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, specularTexDataForPreview.GetTexId());
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTextureId);
		if (modelPreviewObj != nullptr)
			modelPreviewObj->draw();
		glActiveTexture(GL_TEXTURE0);

		gridLineShader.use();
		gridLineShader.applyShaderUniformMatrix(gridLineModelMatrixUniform, glm::scale(glm::mat4(), glm::vec3(100, 0, 100)));
		gridLineShader.applyShaderUniformMatrix(gridLineViewMatrixUniform, glm::lookAt(cameraPosition, glm::vec3(0), glm::vec3(0, 1, 0)));
		gridLineShader.applyShaderUniformMatrix(gridLineProjectionMatrixUniform, glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f));
		previewPlane->draw();

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		if (windowSys.GetWindowRes().x < windowSys.GetWindowRes().y)
		{
			glm::vec2 heightRes = heightMapTexData.getRes();
			brushPanel.getTransform()->setScale(glm::vec2((brushData.brushScale / heightRes.x),
				(brushData.brushScale / heightRes.y)*aspectRatio) * normalViewStateUtility.zoomLevel * 2.0f);

			if (heightRes.x > heightRes.y)
				brushPanel.getTransform()->setScale(brushPanel.getTransform()->getScale() * glm::vec2(heightRes.x / heightRes.y, 1));
			else
				brushPanel.getTransform()->setScale(brushPanel.getTransform()->getScale() * glm::vec2(1, heightRes.y / heightRes.x));
		}
		else
		{
			glm::vec2 heightRes = heightMapTexData.getRes();
			brushPanel.getTransform()->setScale(glm::vec2((brushData.brushScale / heightRes.x) / aspectRatio,
				(brushData.brushScale / heightRes.y)) * normalViewStateUtility.zoomLevel * 2.0f);

			if (heightRes.x > heightRes.y)
				brushPanel.getTransform()->setScale(brushPanel.getTransform()->getScale() * glm::vec2(heightMapTexData.getRes().x / heightMapTexData.getRes().y, 1));
			else
				brushPanel.getTransform()->setScale(brushPanel.getTransform()->getScale() * glm::vec2(1, heightMapTexData.getRes().y / heightMapTexData.getRes().x));
		}
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		brushPreviewShader.use();
		brushPanel.getTransform()->setPosition(((curMouseCoord.x / windowSys.GetWindowRes().x)*2.0f) - 1.0f,
			-(((curMouseCoord.y / windowSys.GetWindowRes().y)*2.0f) - 1.0f));
		brushPanel.getTransform()->update();
		brushPreviewShader.applyShaderFloat(brushPreviewStrengthUniform, brushData.brushStrength);
		brushPreviewShader.applyShaderFloat(brushPreviewOffsetUniform, glm::pow(brushData.brushOffset, 2) * 10.0f);
		brushPreviewShader.applyShaderVector3(brushPreviewColourUniform, (brushData.heightMapPositiveDir ? glm::vec3(1) : glm::vec3(0)));
		brushPreviewShader.applyShaderBool(brushPreviewUseTextureUniform, !brushData.hasBrushTexture());
		brushPreviewShader.applyShaderUniformMatrix(brushPreviewModelUniform, brushPanel.getTransform()->getMatrix());

		brushPanel.setTextureID(brushData.textureData.GetTexId());
		brushPanel.draw();

		ImGui_ImplOpenGL2_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		DisplayWindowTopBar(minimizeTextureId, restoreTextureId, isMaximized, closeTextureId);

		ImGuiWindowFlags window_flags = 0;
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 1.0f);
		window_flags = 0;
		window_flags |= ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoResize;
		window_flags |= ImGuiWindowFlags_NoCollapse;
		window_flags |= ImGuiWindowFlags_NoTitleBar;

		DisplayBottomBar(window_flags);
		DisplaySideBar(window_flags, frameDrawingPanel, saveLocation, shouldSaveNormalMap, changeSize);
		DisplayBrushSettingsUserInterface(isBlurOn);

		if (normalViewStateUtility.mapDrawViewMode < 3)
		{
			DisplayNormalSettingsUserInterface();
			if (normalViewStateUtility.mapDrawViewMode == 2)
				DisplayLightSettingsUserInterface();
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
		DisplayPreview(window_flags);
		fileExplorer.display();
		modalWindow.display();

		//static bool ff = true;
		//ImGui::ShowDemoWindow(&ff);

		ImGui::Render();

		glBindVertexArray(0);
		glUseProgram(0);
		ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
		windowSys.UpdateWindow();
	}
	//applyPanelChangeThread.join();

	delete modelPreviewObj;
	delete cubeForSkybox;
	delete previewPlane;
	delete[] themeItems;

	ImGui_ImplOpenGL2_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	windowSys.Destroy();
	return 0;
}
inline void DisplaySideBar(const ImGuiWindowFlags &window_flags, DrawingPanel &frameDrawingPanel, char saveLocation[500], bool &shouldSaveNormalMap, bool &changeSize)
{
	bool open = true;
	ImGui::SetNextWindowPos(ImVec2(windowSys.GetWindowRes().x - 5, 42), ImGuiSetCond_Always);
	ImGui::SetNextWindowSize(ImVec2(10, windowSys.GetWindowRes().y - 67), ImGuiSetCond_Always);
	ImGui::Begin("Side_Bar", &open, window_flags);
	ImGui::End();

	ImGui::SetNextWindowPos(ImVec2(0, 42), ImGuiSetCond_Always);
	ImGui::SetNextWindowSize(ImVec2(glm::clamp(windowSys.GetWindowRes().x * 0.15f, 280.0f, 600.0f), windowSys.GetWindowRes().y - 77), ImGuiSetCond_Always);
	ImGui::Begin("Settings", &open, window_flags);
	ImGui::PushStyleColor(ImGuiCol_Button, themeManager.SecondaryColour);
	float buttonWidth = ImGui::GetContentRegionAvailWidth() / 3.5f;
	if (ImGui::ImageButton((ImTextureID)toggleFullscreenTexId, ImVec2(buttonWidth, 40), ImVec2(-0.4f, 1.0f), ImVec2(1.4f, 0.0f), -1, ImVec4(0, 0, 0, 0), themeManager.AccentColour2))
	{
		if (!windowSys.IsFullscreen())
			windowSys.SetFullscreen(true);
		else
			windowSys.SetFullscreen(false);
	}
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Toggle Fullscreen (Ctrl + T)");
	ImGui::SameLine();
	//ImGui::Spacing();
	if (ImGui::ImageButton((ImTextureID)clearViewTexId, ImVec2(buttonWidth, 40), ImVec2(-0.4f, 1.0f), ImVec2(1.4f, 0.0f), -1, ImVec4(0, 0, 0, 0), themeManager.AccentColour2))
	{
		std::memset(heightMapTexData.getTextureData(), 255, heightMapTexData.getRes().y * heightMapTexData.getRes().x * heightMapTexData.getComponentCount());
		undoRedoSystem.record(heightMapTexData.getTextureData());
		heightMapTexData.setTextureDirty();
	}
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Clear the panel (Ctrl + Alt + V)");
	ImGui::SameLine();
	if (ImGui::ImageButton((ImTextureID)resetViewTexId, ImVec2(buttonWidth, 40), ImVec2(-0.4f, 0.0f), ImVec2(1.4f, 1.0f), -1, ImVec4(0, 0, 0, 0), themeManager.AccentColour2))
	{
		frameDrawingPanel.getTransform()->setPosition(0, 0);
		normalViewStateUtility.zoomLevel = 1.0f;
	}
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Reset position and scale of panel (Ctrl + V)");
	ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth() * 0.5f);

	ImGui::Spacing();
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 5));
	ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth() / 1.45f);
	ImGui::InputText("## Save location", saveLocation, 500);
	ImGui::PopItemWidth();
	ImGui::SameLine(0, 5);
	if (ImGui::Button("EXPORT", ImVec2(ImGui::GetContentRegionAvailWidth(), 27))) { shouldSaveNormalMap = true; changeSize = true; }
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("[.png | .tga | .bmp | .jpg] Save current view-mode image to file (F10)");
	ImGui::PopStyleVar();
	ImGui::PopStyleColor();
	ImGui::Spacing();
	ImGui::Text("VIEW MODE");
	ImGui::Separator();
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 10));
	int modeButtonWidth = (int)(ImGui::GetContentRegionAvailWidth() / 3.0f);
	ImGui::Spacing();

	if (normalViewStateUtility.mapDrawViewMode == 3) ImGui::PushStyleColor(ImGuiCol_Button, themeManager.AccentColour1);
	else ImGui::PushStyleColor(ImGuiCol_Button, themeManager.SecondaryColour);
	if (ImGui::Button("Height", ImVec2(modeButtonWidth - 5, 40))) { normalViewStateUtility.mapDrawViewMode = 3; }
	ImGui::PopStyleColor();
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("(Ctrl + H)");

	ImGui::SameLine(0, 5);
	if (normalViewStateUtility.mapDrawViewMode == 1) ImGui::PushStyleColor(ImGuiCol_Button, themeManager.AccentColour1);
	else ImGui::PushStyleColor(ImGuiCol_Button, themeManager.SecondaryColour);
	if (ImGui::Button("Normal", ImVec2(modeButtonWidth - 5, 40))) { normalViewStateUtility.mapDrawViewMode = 1; }
	ImGui::PopStyleColor();
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("(Ctrl + J)");

	ImGui::SameLine(0, 5);
	if (normalViewStateUtility.mapDrawViewMode == 2) ImGui::PushStyleColor(ImGuiCol_Button, themeManager.AccentColour1);
	else ImGui::PushStyleColor(ImGuiCol_Button, themeManager.SecondaryColour);
	if (ImGui::Button("Lighting", ImVec2(modeButtonWidth, 40))) { normalViewStateUtility.mapDrawViewMode = 2; }
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("(Ctrl + K)");
	ImGui::PopStyleColor();
	ImGui::PopStyleVar();
}
void DisplayBottomBar(const ImGuiWindowFlags &window_flags)
{
	ImGui::SetNextWindowPos(ImVec2(0, windowSys.GetWindowRes().y - 35), ImGuiSetCond_Always);
	ImGui::SetNextWindowSize(ImVec2(windowSys.GetWindowRes().x, 50), ImGuiSetCond_Always);
	bool open = true;
	ImGui::Begin("Bottom_Bar", &open, window_flags);
	ImGui::Indent(ImGui::GetContentRegionAvailWidth()*0.5f - 30);
	ImGui::Text(VERSION_NAME.c_str());
	ImGui::SameLine();

	static int currentSection = undoRedoSystem.getCurrentSectionPosition();
	static int prevSection = currentSection;
	static int prevUndoSectionPosition;

	ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvailWidth() - 275, 0));
	ImGui::SameLine();
	ImGui::PushStyleVar(ImGuiStyleVar_GrabRounding, 25.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 15.0f);
	ImGui::PushItemWidth(250);
	if (ImGui::SliderInt("##Undo/Redo", &currentSection, 0, undoRedoSystem.getMaxSectionsFilled() - 1, "%d Steps"))
	{
		bool isForward = (currentSection - prevSection) >= 0 ? false : true;
		int count = glm::abs(currentSection - prevSection);
		for (int i = 0; i < count; i++)
			heightMapTexData.updateTextureData(undoRedoSystem.retrieve(isForward));
		heightMapTexData.updateTexture();
		prevSection = currentSection;
	}

	if (undoRedoSystem.getCurrentSectionPosition() >= prevUndoSectionPosition)
	{
		currentSection = undoRedoSystem.getCurrentSectionPosition();
		prevUndoSectionPosition = currentSection;
	}

	ImGui::PopItemWidth();
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Undo/Redo Slider");
	ImGui::PopStyleVar();
	ImGui::PopStyleVar();

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
	glViewport(0, 0, heightMapTexData.getRes().x, heightMapTexData.getRes().y);
	fbs.updateTextureDimensions(heightMapTexData.getRes().x, heightMapTexData.getRes().y);
}
void HandleKeyboardInput(double deltaTime, DrawingPanel &frameDrawingPanel, bool &isMaximized)
{
	//Normal map strength and zoom controls for normal map
	if (windowSys.IsKeyPressed(GLFW_KEY_LEFT) && windowSys.IsKeyPressed(GLFW_KEY_LEFT_ALT) && !windowSys.IsKeyPressed(GLFW_KEY_LEFT_SHIFT) && !windowSys.IsKeyPressed(GLFW_KEY_LEFT_CONTROL))
		normalViewStateUtility.normalMapStrength -= 0.05f;
	if (windowSys.IsKeyPressed(GLFW_KEY_RIGHT) && windowSys.IsKeyPressed(GLFW_KEY_LEFT_ALT) && !windowSys.IsKeyPressed(GLFW_KEY_LEFT_SHIFT) && !windowSys.IsKeyPressed(GLFW_KEY_LEFT_CONTROL))
		normalViewStateUtility.normalMapStrength += 0.05f;
	if (windowSys.IsKeyPressed(GLFW_KEY_W))
		normalViewStateUtility.zoomLevel += normalViewStateUtility.zoomLevel * 1.5f * deltaTime;
	if (windowSys.IsKeyPressed(GLFW_KEY_S))
		normalViewStateUtility.zoomLevel -= normalViewStateUtility.zoomLevel * 1.5f * deltaTime;
	normalViewStateUtility.zoomLevel = glm::clamp(normalViewStateUtility.zoomLevel, 0.1f, 5.0f);

	//Normal map channel toggles
	if (windowSys.IsKeyPressedDown(GLFW_KEY_R) && windowSys.IsKeyPressed(GLFW_KEY_LEFT_SHIFT))
		normalViewStateUtility.redChannelActive = !normalViewStateUtility.redChannelActive;
	if (windowSys.IsKeyPressedDown(GLFW_KEY_G) && windowSys.IsKeyPressed(GLFW_KEY_LEFT_SHIFT))
		normalViewStateUtility.greenChannelActive = !normalViewStateUtility.greenChannelActive;
	if (windowSys.IsKeyPressedDown(GLFW_KEY_B) && windowSys.IsKeyPressed(GLFW_KEY_LEFT_SHIFT))
		normalViewStateUtility.blueChannelActive = !normalViewStateUtility.blueChannelActive;

	//Set normal map view mode in editor
	if (windowSys.IsKeyPressed(GLFW_KEY_H) && windowSys.IsKeyPressed(GLFW_KEY_LEFT_CONTROL))
		normalViewStateUtility.mapDrawViewMode = 3;
	if (windowSys.IsKeyPressed(GLFW_KEY_J) && windowSys.IsKeyPressed(GLFW_KEY_LEFT_CONTROL))
		normalViewStateUtility.mapDrawViewMode = 1;
	if (windowSys.IsKeyPressed(GLFW_KEY_K) && windowSys.IsKeyPressed(GLFW_KEY_LEFT_CONTROL))
		normalViewStateUtility.mapDrawViewMode = 2;

	//Set normal map view mode in model preview
	if (windowSys.IsKeyPressed(GLFW_KEY_H) && windowSys.IsKeyPressed(GLFW_KEY_LEFT_ALT))
		previewStateUtility.modelViewMode = 3;
	if (windowSys.IsKeyPressed(GLFW_KEY_J) && windowSys.IsKeyPressed(GLFW_KEY_LEFT_ALT))
		previewStateUtility.modelViewMode = 1;
	if (windowSys.IsKeyPressed(GLFW_KEY_K) && windowSys.IsKeyPressed(GLFW_KEY_LEFT_ALT))
		previewStateUtility.modelViewMode = 2;
	if (windowSys.IsKeyPressed(GLFW_KEY_L) && windowSys.IsKeyPressed(GLFW_KEY_LEFT_ALT))
		previewStateUtility.modelViewMode = 4;

	//Normal map movement
	if (windowSys.IsKeyPressed(GLFW_KEY_LEFT) && !windowSys.IsKeyPressed(GLFW_KEY_LEFT_SHIFT) && !windowSys.IsKeyPressed(GLFW_KEY_LEFT_ALT) && !windowSys.IsKeyPressed(GLFW_KEY_LEFT_CONTROL))
		frameDrawingPanel.getTransform()->translate(-1.0f * deltaTime, 0.0f);
	if (windowSys.IsKeyPressed(GLFW_KEY_RIGHT) && !windowSys.IsKeyPressed(GLFW_KEY_LEFT_SHIFT) && !windowSys.IsKeyPressed(GLFW_KEY_LEFT_ALT) && !windowSys.IsKeyPressed(GLFW_KEY_LEFT_CONTROL))
		frameDrawingPanel.getTransform()->translate(1.0f * deltaTime, 0);
	if (windowSys.IsKeyPressed(GLFW_KEY_UP) && !windowSys.IsKeyPressed(GLFW_KEY_LEFT_SHIFT) && !windowSys.IsKeyPressed(GLFW_KEY_LEFT_ALT && !windowSys.IsKeyPressed(GLFW_KEY_LEFT_CONTROL)))
		frameDrawingPanel.getTransform()->translate(0.0f, 1.0f * deltaTime);
	if (windowSys.IsKeyPressed(GLFW_KEY_DOWN) && !windowSys.IsKeyPressed(GLFW_KEY_LEFT_SHIFT) && !windowSys.IsKeyPressed(GLFW_KEY_LEFT_ALT && !windowSys.IsKeyPressed(GLFW_KEY_LEFT_CONTROL)))
		frameDrawingPanel.getTransform()->translate(0.0f, -1.0f * deltaTime);

	//Undo
	if (windowSys.IsKeyPressedDown(GLFW_KEY_Z) && windowSys.IsKeyPressed(GLFW_KEY_LEFT_CONTROL))
	{
		heightMapTexData.updateTextureData(undoRedoSystem.retrieve());
		heightMapTexData.updateTexture();
		if (undoRedoSystem.getCurrentSectionPosition() == 0)
			undoRedoSystem.record(heightMapTexData.getTextureData());
	}
	//Redo
	if (windowSys.IsKeyPressedDown(GLFW_KEY_Y) && windowSys.IsKeyPressed(GLFW_KEY_LEFT_CONTROL))
	{
		heightMapTexData.updateTextureData(undoRedoSystem.retrieve(false));
		heightMapTexData.updateTexture();
		if (undoRedoSystem.getCurrentSectionPosition() == 0)
			undoRedoSystem.record(heightMapTexData.getTextureData());
	}
	//Open new image
	if (windowSys.IsKeyPressedDown(GLFW_KEY_O) && windowSys.IsKeyPressed(GLFW_KEY_LEFT_CONTROL))
	{
		currentLoadingOption = LoadingOption::TEXTURE;
		fileExplorer.displayDialog(FileType::IMAGE, [&](std::string str)
		{
			if (currentLoadingOption == LoadingOption::TEXTURE)
			{
				TextureManager::getTextureDataFromFile(str, heightMapTexData);
				heightMapTexData.SetTexId(TextureManager::loadTextureFromData(heightMapTexData));
				heightMapTexData.setTextureDirty();
				normalmapPanel.setTextureID(heightMapTexData.GetTexId());

				undoRedoSystem.updateAllocation(heightMapTexData.getRes(), heightMapTexData.getComponentCount(), 20);
				undoRedoSystem.record(heightMapTexData.getTextureData());
			}
		});
	}

	//Brush data
	if (windowSys.IsKeyPressed(GLFW_KEY_UP) && windowSys.IsKeyPressed(GLFW_KEY_LEFT_SHIFT) && !windowSys.IsKeyPressed(GLFW_KEY_LEFT_CONTROL))
		brushData.brushScale += 0.1f;
	if (windowSys.IsKeyPressed(GLFW_KEY_DOWN) && windowSys.IsKeyPressed(GLFW_KEY_LEFT_SHIFT) && !windowSys.IsKeyPressed(GLFW_KEY_LEFT_CONTROL))
		brushData.brushScale -= 0.1f;
	if (windowSys.IsKeyPressed(GLFW_KEY_RIGHT) && windowSys.IsKeyPressed(GLFW_KEY_LEFT_SHIFT) && !windowSys.IsKeyPressed(GLFW_KEY_LEFT_CONTROL))
		brushData.brushOffset += 0.01f;
	if (windowSys.IsKeyPressed(GLFW_KEY_LEFT) && windowSys.IsKeyPressed(GLFW_KEY_LEFT_SHIFT) && !windowSys.IsKeyPressed(GLFW_KEY_LEFT_CONTROL))
		brushData.brushOffset -= 0.01f;
	if (windowSys.IsKeyPressed(GLFW_KEY_UP) && windowSys.IsKeyPressed(GLFW_KEY_LEFT_SHIFT) && windowSys.IsKeyPressed(GLFW_KEY_LEFT_CONTROL))
		brushData.brushStrength += 0.01f;
	if (windowSys.IsKeyPressed(GLFW_KEY_DOWN) && windowSys.IsKeyPressed(GLFW_KEY_LEFT_SHIFT) && windowSys.IsKeyPressed(GLFW_KEY_LEFT_CONTROL))
		brushData.brushStrength -= 0.01f;
	brushData.brushOffset = glm::clamp(brushData.brushOffset, 0.01f, 1.0f);
	brushData.brushScale = glm::clamp(brushData.brushScale, 1.0f, heightMapTexData.getRes().y * 0.5f);
	brushData.brushStrength = glm::clamp(brushData.brushStrength, 0.0f, 1.0f);

	//Flip normal map x-y axis
	if (windowSys.IsKeyPressedDown(GLFW_KEY_A) && windowSys.IsKeyPressed(GLFW_KEY_LEFT_CONTROL))
		normalViewStateUtility.flipX_Ydir = !normalViewStateUtility.flipX_Ydir;

	//Window fullscreen toggle
	if (windowSys.IsKeyPressedDown(GLFW_KEY_T) && windowSys.IsKeyPressed(GLFW_KEY_LEFT_CONTROL))
	{
		if (!windowSys.IsFullscreen())
			windowSys.SetFullscreen(true);
		else
			windowSys.SetFullscreen(false);
	}

	//Normal map panel reset/clear
	if (windowSys.IsKeyPressedDown(GLFW_KEY_V) && windowSys.IsKeyPressed(GLFW_KEY_LEFT_CONTROL))
	{
		//clear
		if (windowSys.IsKeyPressed(GLFW_KEY_LEFT_ALT))
		{
			std::memset(heightMapTexData.getTextureData(), 255, heightMapTexData.getRes().y * heightMapTexData.getRes().x * heightMapTexData.getComponentCount());
			undoRedoSystem.record(heightMapTexData.getTextureData());
			heightMapTexData.setTextureDirty();
		}
		//reset
		else
		{
			frameDrawingPanel.getTransform()->setPosition(0, 0);
			normalViewStateUtility.zoomLevel = 1;
		}
	}

	//Minimize window
	if (windowSys.IsKeyPressed(GLFW_KEY_F9))
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

	static std::string currentBrush = "Circle";
	static std::vector<std::string> grungeBrushPaths = fileExplorer.getAllFilesInDirectory(BRUSH_TEXTURES_PATH + "Grunge", false);
	static std::vector<std::string> patternBrushPaths = fileExplorer.getAllFilesInDirectory(BRUSH_TEXTURES_PATH + "Patterns", false);

	if (ImGui::BeginMenu(currentBrush.c_str()))
	{
		if (ImGui::BeginMenu("Geometric"))
		{
			if (ImGui::MenuItem("Circle")) { brushData.textureData.clearRawData(); }
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Grunge"))
		{
			for (int i = 0; i < grungeBrushPaths.size(); i++)
			{
				if (ImGui::MenuItem(grungeBrushPaths[i].c_str()))
				{
					currentBrush = grungeBrushPaths[i];
					TextureManager::getTextureDataFromFile(BRUSH_TEXTURES_PATH + "Grunge\\" + grungeBrushPaths[i], brushData.textureData);
					brushData.textureData.SetTexId(TextureManager::loadTextureFromData(brushData.textureData));
				}
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Pattern"))
		{
			for (int i = 0; i < patternBrushPaths.size(); i++)
			{
				if (ImGui::MenuItem(patternBrushPaths[i].c_str()))
				{
					currentBrush = grungeBrushPaths[i];
					TextureManager::getTextureDataFromFile(BRUSH_TEXTURES_PATH + "Patterns\\" + patternBrushPaths[i], brushData.textureData);
					brushData.textureData.SetTexId(TextureManager::loadTextureFromData(brushData.textureData));
				}
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenu();
	}
	static int item_type;
	ImGui::RadioButton("HEIGHT MODE", &item_type, 0); ImGui::SameLine();
	ImGui::RadioButton("BLUR MODE", &item_type, 1);
	/*if (ImGui::Button((isBlurOn) ? "HEIGHT MODE" : "BLUR MODE", ImVec2((int)(ImGui::GetContentRegionAvailWidth() / 2.0f), 40)))
		isBlurOn = !isBlurOn;
	ImGui::SameLine();*/
	isBlurOn = (item_type == 0) ? false : true;
	if (isBlurOn)
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);

	if (ImGui::Button((brushData.heightMapPositiveDir) ? "Height -VE" : "Height +VE", ImVec2(ImGui::GetContentRegionAvailWidth(), 40)))
		brushData.heightMapPositiveDir = (isBlurOn) ? brushData.heightMapPositiveDir : !brushData.heightMapPositiveDir;
	if (isBlurOn)
		ImGui::PopStyleVar();
	ImGui::PopStyleColor();
	if (ImGui::SliderFloat(" Brush Scale", &brushData.brushScale, 1.0f, heightMapTexData.getRes().y*0.5f, "%.2f", 1.0f)) {}
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("(Shift + |UP / DOWN|)");
	if (ImGui::SliderFloat(" Brush Offset", &brushData.brushOffset, 0.01f, 1.0f, "%.2f", 1.0f)) {}
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("(Shift + |LEFT / RIGHT|)");
	if (ImGui::SliderFloat(" Brush Strength", &brushData.brushStrength, 0.0f, 1.0f, "%0.2f", 1.0f)) {}
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("(Shift + Ctrl + |UP / DOWN|)");
	if (!isBlurOn)
	{
		if (ImGui::SliderFloat(" Brush Min Height", &brushData.brushMinHeight, 0.0f, 1.0f, "%0.2f", 1.0f)) {}
		if (ImGui::SliderFloat(" Brush Max Height", &brushData.brushMaxHeight, 0.0f, 1.0f, "%0.2f", 1.0f)) {}
	}
	if (ImGui::SliderFloat(" Brush Draw Rate", &brushData.brushRate, 0.0f, heightMapTexData.getRes().y / 2, "%0.2f", 1.0f)) {}
	static BrushData bCopy = BrushData();
	if (bCopy != brushData)
		bCopy = brushData;

	if (brushData.brushMinHeight > brushData.brushMaxHeight)
		brushData.brushMinHeight = brushData.brushMaxHeight;
	else if (brushData.brushMaxHeight < brushData.brushMinHeight)
		brushData.brushMaxHeight = brushData.brushMinHeight;
}
inline void DisplayNormalSettingsUserInterface()
{
	ImGui::Text("NORMAL SETTINGS");
	ImGui::Separator();

	const char* items[] = { "Tri-Sample", "Sobel" };
	static int item_current = 0;
	ImGui::Combo("##combo", &item_current, items, IM_ARRAYSIZE(items));
	switch (item_current)
	{
	case 0:
		normalViewStateUtility.methodIndex = 0;
		break;
	case 1:
		normalViewStateUtility.methodIndex = 1;
		break;
	default:
		break;
	}
	ImGui::SameLine();
	ImGui::Text("Method");

	if (ImGui::SliderFloat(" Normal Strength", &normalViewStateUtility.normalMapStrength, -10.0f, 10.0f, "%.2f")) {}
	ImGui::PushStyleColor(ImGuiCol_Button, themeManager.SecondaryColour);
	if (ImGui::Button("Flip X-Y", ImVec2(ImGui::GetContentRegionAvailWidth(), 40))) { normalViewStateUtility.flipX_Ydir = !normalViewStateUtility.flipX_Ydir; }
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("(CTRL + A)");
	ImGui::PopStyleColor();
	const float width = ImGui::GetContentRegionAvailWidth() / 3.0f - 7;

	if (!normalViewStateUtility.redChannelActive) ImGui::PushStyleColor(ImGuiCol_Button, themeManager.AccentColour1);
	else ImGui::PushStyleColor(ImGuiCol_Button, themeManager.SecondaryColour);
	if (ImGui::Button("R", ImVec2(width, 40))) { normalViewStateUtility.redChannelActive = !normalViewStateUtility.redChannelActive; } ImGui::SameLine();
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("(SHIFT + R)");
	ImGui::PopStyleColor();

	if (!normalViewStateUtility.greenChannelActive) ImGui::PushStyleColor(ImGuiCol_Button, themeManager.AccentColour1);
	else ImGui::PushStyleColor(ImGuiCol_Button, themeManager.SecondaryColour);
	if (ImGui::Button("G", ImVec2(width, 40))) { normalViewStateUtility.greenChannelActive = !normalViewStateUtility.greenChannelActive; } ImGui::SameLine();
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("(SHIFT + G)");
	ImGui::PopStyleColor();

	if (!normalViewStateUtility.blueChannelActive) ImGui::PushStyleColor(ImGuiCol_Button, themeManager.AccentColour1);
	else ImGui::PushStyleColor(ImGuiCol_Button, themeManager.SecondaryColour);
	if (ImGui::Button("B", ImVec2(width, 40))) { normalViewStateUtility.blueChannelActive = !normalViewStateUtility.blueChannelActive; }
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("(SHIFT + B)");
	ImGui::PopStyleColor();
}

inline void DisplayLightSettingsUserInterface()
{
	ImGui::Text("LIGHT SETTINGS");
	ImGui::Separator();
	if (ImGui::SliderFloat(" Diffuse Intensity", &normalViewStateUtility.lightIntensity, 0.0f, 1.0f, "%.2f")) {}
	if (ImGui::SliderFloat(" Specularity", &normalViewStateUtility.specularity, 1.0f, 100.0f, "%.2f")) {}
	if (ImGui::SliderFloat(" Specularity Strength", &normalViewStateUtility.specularityStrength, 0.0f, 10.0f, "%.2f")) {}
	ImGui::Text("Light Direction");
	ImGui::PushItemWidth((ImGui::GetContentRegionAvailWidth() / 3.0f) - 7);
	if (ImGui::SliderFloat("## X Angle", &normalViewStateUtility.lightDirection.x, 0.01f, 359.99f, "X:%.2f")) {}
	ImGui::SameLine();
	if (ImGui::SliderFloat("## Y Angle", &normalViewStateUtility.lightDirection.y, 0.01f, 359.99f, "Y:%.2f")) {}
	ImGui::SameLine();
	if (ImGui::SliderFloat("## Z Angle", &normalViewStateUtility.lightDirection.z, 0.01f, 359.99f, "Z:%.2f")) {}
	ImGui::PopItemWidth();
}

inline void DisplayPreview(const ImGuiWindowFlags &window_flags)
{
	bool open = true;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 1.0f);
	ImGui::PushStyleColor(ImGuiCol_SliderGrab, themeManager.SecondaryColour);
	ImGui::SetNextWindowPos(ImVec2(windowSys.GetWindowRes().x - 305, 42), ImGuiSetCond_Always);
	ImGui::SetNextWindowSize(ImVec2(300, windowSys.GetWindowRes().y - 77), ImGuiSetCond_Always);
	ImGui::Begin("Preview_Bar", &open, window_flags);
	static bool isPreviewOpen = true;
	if (ImGui::ImageButton((ImTextureID)maximizePreviewTexId, ImVec2(80, 40), ImVec2(-0.45f, 1), ImVec2(1.45f, 0), -1, ImVec4(0, 0, 0, 0), themeManager.AccentColour2))
	{
		isPreviewOpen = true;
		ImGui::OpenPopup("Preview");
	}
	ImGui::SetNextWindowPosCenter();
	ImGui::SetNextWindowSizeConstraints(ImVec2(400, 400), ImVec2(windowSys.GetWindowRes().x * 0.95f, windowSys.GetWindowRes().y * 0.95f));
	if (ImGui::BeginPopupModal("Preview", &isPreviewOpen, ImGuiWindowFlags_NoMove))
	{
		float aspectRatio = 0.0f;
		float width = ImGui::GetContentRegionAvail().x;
		float height = ImGui::GetContentRegionAvail().y;

		aspectRatio = width / height;
		if (width > height)
			width /= aspectRatio;
		else
			height *= aspectRatio;
		if (ImGui::GetWindowWidth() > ImGui::GetWindowHeight())
		{
			ImGui::Dummy(ImVec2(((ImGui::GetContentRegionAvail().x - width) * 0.5f) - 10, 0));
			ImGui::SameLine();
			ImGui::Image((ImTextureID)previewFbs.getColourTexture(), ImVec2(width, height));
		}
		else
		{
			ImGui::Dummy(ImVec2(((ImGui::GetContentRegionAvail().x - width) * 0.5f) - 10, (ImGui::GetContentRegionAvail().y - height) * 0.5f));
			ImGui::Image((ImTextureID)previewFbs.getColourTexture(), ImVec2(width, height));
		}
		ImGui::EndPopup();
	}
	ImGui::SameLine();
	const char* items[] = { "    CUBE", "    CYLINDER", "    SPHERE", "    TORUS", "    CUSTOM MODEL" };
	static const char* current_item = items[0];
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 15));
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
	ImGui::PopStyleVar();
	ImGui::Spacing();
	ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth() + 5);
	ImGui::Image((ImTextureID)previewFbs.getColourTexture(), ImVec2(300, 300));
	ImGui::SliderFloat("##Zoom level", &previewStateUtility.modelPreviewZoomLevel, -1.0f, -100.0f, "Zoom Level:%.2f");
	ImGui::SliderFloat("##Roughness", &previewStateUtility.modelRoughness, 0.0f, 10.0f, "Roughness:%.2f");
	ImGui::SliderFloat("##Reflectivity", &previewStateUtility.modelReflectivity, 0.0f, 1.0f, "Reflectivity:%.2f");
	ImGui::PopItemWidth();
	ImGui::Spacing();
	ImGui::Text("VIEW MODE");
	ImGui::Separator();
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 10));
	int modeButtonWidth = (int)(ImGui::GetContentRegionAvailWidth() / 4.0f);
	ImGui::Spacing();

	if (previewStateUtility.modelViewMode == 3) ImGui::PushStyleColor(ImGuiCol_Button, themeManager.AccentColour1);
	else ImGui::PushStyleColor(ImGuiCol_Button, themeManager.SecondaryColour);
	if (ImGui::Button("Height", ImVec2(modeButtonWidth - 5, 40))) { previewStateUtility.modelViewMode = 3; }
	ImGui::PopStyleColor();
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("(Alt + H)");

	ImGui::SameLine(0, 5);
	if (previewStateUtility.modelViewMode == 1) ImGui::PushStyleColor(ImGuiCol_Button, themeManager.AccentColour1);
	else ImGui::PushStyleColor(ImGuiCol_Button, themeManager.SecondaryColour);
	if (ImGui::Button("Normal", ImVec2(modeButtonWidth - 5, 40))) { previewStateUtility.modelViewMode = 1; }
	ImGui::PopStyleColor();
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("(Alt + J)");

	ImGui::SameLine(0, 5);
	if (previewStateUtility.modelViewMode == 2) ImGui::PushStyleColor(ImGuiCol_Button, themeManager.AccentColour1);
	else ImGui::PushStyleColor(ImGuiCol_Button, themeManager.SecondaryColour);
	if (ImGui::Button("Lighting", ImVec2(modeButtonWidth, 40))) { previewStateUtility.modelViewMode = 2; }
	ImGui::PopStyleColor();
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("(Alt + K)");

	ImGui::SameLine(0, 5);
	if (previewStateUtility.modelViewMode == 4) ImGui::PushStyleColor(ImGuiCol_Button, themeManager.AccentColour1);
	else ImGui::PushStyleColor(ImGuiCol_Button, themeManager.SecondaryColour);
	if (ImGui::Button("Textured", ImVec2(modeButtonWidth, 40))) { previewStateUtility.modelViewMode = 4; }
	ImGui::PopStyleColor();
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("(Alt + L)");

	ImGui::PopStyleVar();

	if (previewStateUtility.modelViewMode == 2 || previewStateUtility.modelViewMode == 4)
	{
		ImGui::Text("LIGHTING SETTINGS");
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::SliderAngle("Position", &previewStateUtility.lightLocation, 0.0f, 360.0f);
		ImGui::SliderFloat("Intensity", &previewStateUtility.lightIntensity, 0.0f, 10.0f, "%.2f");
		ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth() + 5);
		ImGui::Text("Light Colour"); ImGui::SameLine();
		ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvailWidth() - 40, 10)); ImGui::SameLine();
		ImGui::ColorEdit3("Light Color", &previewStateUtility.lightColour[0], ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoOptions);
		ImGui::Text("Diffuse Colour"); ImGui::SameLine();
		ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvailWidth() - 40, 10)); ImGui::SameLine();
		ImGui::ColorEdit3("Diffuse Color", &previewStateUtility.diffuseColour[0], ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoOptions);
		ImGui::PopItemWidth();

		if (previewStateUtility.modelViewMode == 4)
		{
			ImGui::Spacing();
			ImGui::Text("TEXTURE SETTINGS");
			ImGui::Separator();
			ImGui::Spacing();
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
			ImGui::Text("Diffuse / Albedo");
			ImGui::SameLine(0, 5);
			ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvailWidth() - 60, 10)); ImGui::SameLine();
			if (ImGui::ImageButton((ImTextureID)diffuseTexDataForPreview.GetTexId(), ImVec2(40, 40)))
			{
				currentLoadingOption = LoadingOption::TEXTURE;
				fileExplorer.displayDialog(FileType::IMAGE, [&](std::string str)
				{
					if (currentLoadingOption == LoadingOption::TEXTURE)
						diffuseTexDataForPreview.SetTexId(TextureManager::loadTextureFromFile(str));
				});
			}
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Load diffuse texture for preview model");

			ImGui::Text("Specular");
			ImGui::SameLine(0, 5);
			ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvailWidth() - 60, 10)); ImGui::SameLine();
			if (ImGui::ImageButton((ImTextureID)specularTexDataForPreview.GetTexId(), ImVec2(40, 40)))
			{
				currentLoadingOption = LoadingOption::TEXTURE;
				fileExplorer.displayDialog(FileType::IMAGE, [&](std::string str)
				{
					if (currentLoadingOption == LoadingOption::TEXTURE)
						specularTexDataForPreview.SetTexId(TextureManager::loadTextureFromFile(str));
				});
			}
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Load specular texture for preview model");

			ImGui::PopStyleVar();
		}
	}
	ImGui::End();

	ImGui::PopStyleColor();
	ImGui::PopStyleVar(3);
}
inline void DisplayWindowTopBar(unsigned int minimizeTexture, unsigned int restoreTexture, bool &isMaximized, unsigned int closeTexture)
{
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 13));
	if (ImGui::BeginMainMenuBar())
	{
		ImGui::Indent(10);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(25, 5));
		ImGui::PushFont(menuBarLargerText);
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open Image", "CTRL+O"))
			{
				currentLoadingOption = LoadingOption::TEXTURE;
				fileExplorer.displayDialog(FileType::IMAGE, [&](std::string str)
				{
					if (currentLoadingOption == LoadingOption::TEXTURE)
					{
						TextureManager::getTextureDataFromFile(str, heightMapTexData);
						heightMapTexData.SetTexId(TextureManager::loadTextureFromData(heightMapTexData));
						heightMapTexData.setTextureDirty();
						normalmapPanel.setTextureID(heightMapTexData.GetTexId());

						undoRedoSystem.updateAllocation(heightMapTexData.getRes(), heightMapTexData.getComponentCount(), 20);
						undoRedoSystem.record(heightMapTexData.getTextureData());
					}
				});
			}
			if (ImGui::MenuItem("Preferences"))
			{
				ImGui::OpenPopup("eww");
			}
			ImGui::EndMenu();
		}
		bool oprn = true;
		ImGui::SetNextWindowSizeConstraints(ImVec2(540, 540), ImVec2(1920, 1080));
		if (ImGui::BeginPopupModal("eww", &oprn, ImGuiWindowFlags_NoScrollbar))
		{
			if (ImGui::Button("CLOSE", ImVec2(80, 30)))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		if (ImGui::BeginMenu("Edit"))
		{
			bool isUndoDisabled = undoRedoSystem.getCurrentSectionPosition() == 1 ? true : false;
			bool isRedoDisabled = undoRedoSystem.getCurrentSectionPosition() >= 1 && undoRedoSystem.getCurrentSectionPosition() <= undoRedoSystem.getMaxSectionsFilled() - 1 ? false : true;

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
				heightMapTexData.updateTextureData(undoRedoSystem.retrieve(false));
				heightMapTexData.updateTexture();
			}
			if (isRedoDisabled)
				ImGui::PopStyleVar();
			ImGui::EndMenu();
		}

		if (themeItems == nullptr)
		{
			themeItems = new char *[themeManager.getNumberOfThemes()];
			themeItems[0] = new char[8]{ 'D','e','f','a', 'u', 'l', 't' };
			themeItems[1] = new char[5]{ 'D','a','r','k' };
			themeItems[2] = new char[6]{ 'L','i','g','h', 't' };

			std::vector<std::string> loadedThemes = themeManager.GetAllLoadedThemes();
			for (int i = 0; i < loadedThemes.size(); i++)
			{
				std::string val = loadedThemes[i];
				themeItems[i + 3] = new char[val.size()];
				std::memcpy(themeItems[i + 3], &val[0], val.size());
				std::memset(themeItems[i + 3] + (val.size()), '\0', 1);
			}
		}

		static int item_current = 0;
		static int prev_item = -1;
		ImGui::PushItemWidth(180);
		ImGui::Combo("##combo", &item_current, themeItems, themeManager.getNumberOfThemes());
		ImGui::PopItemWidth();
		ImGui::PopFont();
		if (prev_item != item_current)
		{
			if (item_current >= 3)
				themeManager.SetThemeFromFile(THEMES_PATH + (std::string(themeItems[item_current]) + ".nort"));
			else
			{
				if (item_current == 0)
					themeManager.EnableInBuiltTheme(ThemeManager::Theme::DEFAULT);
				else if (item_current == 1)
					themeManager.EnableInBuiltTheme(ThemeManager::Theme::DARK);
				else if (item_current == 2)
					themeManager.EnableInBuiltTheme(ThemeManager::Theme::LIGHT);
			}
		}
		prev_item = item_current;
		ImGui::PopStyleVar();
#ifdef NORA_CUSTOM_WINDOW_CHROME
		ImGui::Indent(windowSys.GetWindowRes().x - 160);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(5, 10));
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		//if )
			//ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ACCENT_COL);
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
		//if (isUsingCustomTheme)
			//ImGui::PopStyleColor();
		ImGui::PopStyleVar();
#endif
	}
	ImGui::EndMainMenuBar();
	ImGui::PopStyleVar();
}
void SaveNormalMapToFile(const std::string &locationStr, ImageFormat imageFormat)
{
	if (locationStr.length() > 4)
	{
		fbs.BindColourTexture();
		glPixelStorei(GL_PACK_ALIGNMENT, 1);

		const int nSize = heightMapTexData.getRes().x * heightMapTexData.getRes().y * 3;
		char* dataBuffer = new char[nSize];
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, dataBuffer);

		glBindTexture(GL_TEXTURE_2D, 0);
		fbs.updateTextureDimensions(windowSys.GetWindowRes().x, windowSys.GetWindowRes().y);
		TextureManager::SaveImage(locationStr, heightMapTexData.getRes(), imageFormat, dataBuffer);
		delete[] dataBuffer;
	}
}

inline void HandleLeftMouseButtonInput_NormalMapInteraction(int state, DrawingPanel &frameDrawingPanel, bool isBlurOn)
{
	const glm::vec2 INVALID = glm::vec2(-100000000, -10000000);
	static glm::vec2 prevMouseCoord = INVALID;
	static int prevState = GLFW_RELEASE;
	static bool didActuallyDraw = false;
	const glm::vec2 currentMouseCoord = windowSys.GetCursorPos();

	if (state == GLFW_PRESS && !fileExplorer.shouldDisplay)
	{
		const glm::vec2 wnCurMouse = glm::vec2(currentMouseCoord.x / windowSys.GetWindowRes().x, 1.0f - currentMouseCoord.y / windowSys.GetWindowRes().y);
		const glm::vec2 wnPrevMouse = glm::vec2(prevMouseCoord.x / windowSys.GetWindowRes().x, 1.0f - prevMouseCoord.y / windowSys.GetWindowRes().y);
		//viewport current mouse coords
		const glm::vec2 vpCurMouse(wnCurMouse.x * 2.0f - 1.0f, wnCurMouse.y * 2.0f - 1.0f);
		//viewport previous mouse coords
		const glm::vec2 vpPrevMouse(wnPrevMouse.x * 2.0f - 1.0f, wnPrevMouse.y * 2.0f - 1.0f);

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
				const float curX = (vpCurMouse.x - bottomLeftCorner.x) / glm::abs((topRightCorner.x - bottomLeftCorner.x));
				const float curY = (vpCurMouse.y - bottomLeftCorner.y) / glm::abs((topRightCorner.y - bottomLeftCorner.y));

				const float maxWidth = heightMapTexData.getRes().x;
				const float maxHeight = heightMapTexData.getRes().y;
				glm::vec2 convertedBrushScale(brushData.brushScale / maxWidth, brushData.brushScale / maxHeight);

				if (!isBlurOn)
				{
					float density = 0.01f; //*Think density should axis dependant
					if (distOfPrevAndCurrentMouseCoord > density && prevMouseCoord != INVALID)
					{
						float prevX = (vpPrevMouse.x - bottomLeftCorner.x) / glm::abs((topRightCorner.x - bottomLeftCorner.x));
						float prevY = (vpPrevMouse.y - bottomLeftCorner.y) / glm::abs((topRightCorner.y - bottomLeftCorner.y));

						glm::vec2 prevPoint(prevX, prevY);
						glm::vec2 toPoint(curX, curY);
						glm::vec2 iterCurPoint = prevPoint;

						glm::vec2 diff = (prevPoint - toPoint);
						glm::vec2 incValue = glm::normalize(diff) * density;
						int numberOfPoints = glm::floor(glm::clamp(glm::length(diff) / density, 1.0f, 300.0f));

						for (int i = 0; i < numberOfPoints; i++)
						{
							float left = glm::clamp((iterCurPoint.x - convertedBrushScale.x) * maxWidth, 0.0f, maxWidth);
							float right = glm::clamp((iterCurPoint.x + convertedBrushScale.x) * maxWidth, 0.0f, maxWidth);
							float bottom = glm::clamp((iterCurPoint.y - convertedBrushScale.y) * maxHeight, 0.0f, maxHeight);
							float top = glm::clamp((iterCurPoint.y + convertedBrushScale.y) * maxHeight, 0.0f, maxHeight);
							iterCurPoint += incValue;
							if (brushData.hasBrushTexture())
								SetPixelValuesWithBrushTexture(heightMapTexData, brushData.textureData, left, right, bottom, top, iterCurPoint.x, iterCurPoint.y);
							else
								SetPixelValues(heightMapTexData, left, right, bottom, top, iterCurPoint.x, iterCurPoint.y);
						}
					}
					else
					{
						float left = glm::clamp((curX - convertedBrushScale.x) * maxWidth, 0.0f, maxWidth);
						float right = glm::clamp((curX + convertedBrushScale.x) * maxWidth, 0.0f, maxWidth);
						float bottom = glm::clamp((curY - convertedBrushScale.y) * maxHeight, 0.0f, maxHeight);
						float top = glm::clamp((curY + convertedBrushScale.y) * maxHeight, 0.0f, maxHeight);
						if (brushData.hasBrushTexture())
							SetPixelValuesWithBrushTexture(heightMapTexData, brushData.textureData, left, right, bottom, top, curX, curY);
						else
							SetPixelValues(heightMapTexData, left, right, bottom, top, curX, curY);

					}
				}
				else if (isBlurOn)
				{
					float left = (curX - convertedBrushScale.x) * maxWidth;
					float right = (curX + convertedBrushScale.x) * maxWidth;
					float bottom = (curY - convertedBrushScale.y) * maxHeight;
					float top = (curY + convertedBrushScale.y) * maxHeight;

					if (left >= 0 && right <= maxWidth && top <= maxHeight && bottom >= 0)
					{
						left = glm::clamp(left, 0.0f, maxWidth);
						right = glm::clamp(right, 0.0f, maxWidth);
						bottom = glm::clamp(bottom, 0.0f, maxHeight);
						top = glm::clamp(top, 0.0f, maxHeight);
						SetBluredPixelValues(heightMapTexData, left, right, bottom, top, curX, curY);
					}
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
			prevMouseCoord = INVALID;
			undoRedoSystem.record(heightMapTexData.getTextureData());
			didActuallyDraw = false;
		}
	}
	prevState = state;
}

inline void HandleLeftMouseButtonInput_UI(int state, glm::vec2 &initPos, WindowSide &windowSideAtInitPos, double x, double y, bool &isMaximized, glm::vec2 &prevGlobalFirstMouseCoord)
{
#ifdef NORA_CUSTOM_WINDOW_CHROME 
	if (state == GLFW_PRESS)
	{
		if (initPos == glm::vec2(-1000, -1000))
		{
			windowSideAtInitPos = WindowTransformUtility::GetWindowAreaAtMouseCoord((int)x, (int)y, windowSys.GetWindowRes().x, windowSys.GetWindowRes().y); //WindowTransformUtility::GetWindowSideAtMouseCoord((int)x, (int)y, windowSys.GetWindowRes().x, windowSys.GetWindowRes().y);
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
	const glm::vec2 pixelPos(xpos, ypos);
	const float px_width = inputTexData.getRes().x;
	const float px_height = inputTexData.getRes().y;
	const float distanceRemap = 1.0f / brushData.brushScale;
	const float offsetRemap = glm::pow(brushData.brushOffset, 2) * 10.0f;

	float xMag = endX - startX;
	float yMag = endY - startY;
	for (int i = startX; i < endX; i++)
	{
		for (int j = startY; j < endY; j++)
		{
			ColourData colData = inputTexData.getTexelColor(i, j);
			float rVal = colData.getColour_32_Bit().r;

			float x = (i - startX) / xMag;
			float y = (j - startY) / yMag;

			float distance = glm::distance(glm::vec2(0), glm::vec2(x * 2.0f - 1.0f, y * 2.0f - 1.0f)) * (1.0 / brushData.brushScale);
			if (distance < distanceRemap)
			{
				distance = (1.0f - (distance / distanceRemap)) * offsetRemap;
				distance = glm::clamp(distance, 0.0f, 1.0f) * brushData.brushStrength;
				rVal = rVal + distance * ((brushData.heightMapPositiveDir ? brushData.brushMaxHeight : brushData.brushMinHeight) - rVal);//inputTexData.getTexelColor((int)( x * heightMapTexData.getRes().x),(int)( y * heightMapTexData.getRes().y)).getColour_32_Bit().r;
				ColourData col(rVal, rVal, rVal, 1.0f);
				inputTexData.setTexelColor(col, i, j);
			}
		}
	}
}

inline void SetPixelValuesWithBrushTexture(TextureData& inputTexData, TextureData& brushTexture, int startX, int endX, int startY, int endY, double xpos, double ypos)
{
	float xMag = endX - startX;
	float yMag = endY - startY;

	for (int i = startX; i < endX; i++)
	{
		for (int j = startY; j < endY; j++)
		{
			ColourData colData = inputTexData.getTexelColor(i, j);
			float defVal = colData.getColour_32_Bit().r;
			float finalOutput = defVal;

			float x = (i - startX) / xMag;
			float y = (j - startY) / yMag;

			float brushTexVal = glm::pow(brushTexture.getTexelColor((int)(x * brushTexture.getRes().x), (int)(y * brushTexture.getRes().y)).getColour_32_Bit().r, brushData.brushOffset);
			finalOutput += (brushTexVal * ((brushData.heightMapPositiveDir ? brushData.brushMaxHeight : brushData.brushMinHeight) - finalOutput));
			finalOutput = glm::clamp(finalOutput, 0.0f, 1.0f);
			finalOutput = glm::mix(defVal, finalOutput, brushData.brushStrength);
			ColourData col(finalOutput, finalOutput, finalOutput, 1.0f);
			inputTexData.setTexelColor(col, i, j);
		}
	}
}

inline void SetBluredPixelValues(TextureData& inputTexData, int startX, int endX, int startY, int endY, double xpos, double ypos)
{
	const float distanceRemap = (0.95f / brushData.brushScale) - 0.01f;
	//Temp allocation of image section

	const int _width = endX - startX;
	const int _height = endY - startY;
	const int totalPixelCount = _width * _height;
	ColourData *tempPixelData = new ColourData[totalPixelCount];

	for (int i = startX; i < endX; i++)
	{
		for (int j = startY; j < endY; j++)
		{
			int index = (i - startX) * _width + (j - startY);
			if (index >= 0 && index < totalPixelCount)
				tempPixelData[index] = inputTexData.getTexelColor(i, j);
		}
	}
	float xMag = endX - startX;
	float yMag = endY - startY;

	for (int i = startX; i < endX; i++)
	{
		for (int j = startY; j < endY; j++)
		{
			float x = (i - startX) / xMag;
			float y = (j - startY) / yMag;
			float distance = glm::distance(glm::vec2(0), glm::vec2(x * 2.0f - 1.0f, y * 2.0f - 1.0f));
			distance = glm::clamp(distance, 0.0f, 1.0f);
			if (distance < 0.9f)
			{
				int index = (i - startX) * xMag + (j - startY);
				if (index < 0 || index >= totalPixelCount)
					continue;

				float pixelCol = tempPixelData[index].getColour_32_Bit().r;
				float avg = pixelCol * 0.5f;

				int leftIndex = ((i - 1) - startX) * xMag + (j - startY);
				int rightIndex = ((i + 1) - startX) * xMag + (j - startY);
				int topIndex = (i - startX) * xMag + ((j + 1) - startY);
				int bottomIndex = (i - startX) * xMag + ((j - 1) - startY);

				int topLeftIndex = ((i - 1) - startX) * xMag + ((j + 1) - startY);
				int bottomLeftIndex = ((i - 1) - startX) * xMag + ((j - 1) - startY);
				int topRightIndex = ((i + 1) - startX) * xMag + ((j + 1) - startY);
				int bottomRightIndex = ((i + 1) - startX) * xMag + ((j - 1) - startY);

				int kernel[] = { leftIndex, rightIndex, topIndex, bottomIndex, topLeftIndex, bottomLeftIndex, topRightIndex, bottomRightIndex };
				//not clamping values based in width and heifhgt of current pixel center
				float validEntries = 0;
				float neighbourAvg = 0;
				for (unsigned int i = 0; i < 8; i++)
				{
					if (kernel[i] >= 0 && kernel[i] < totalPixelCount)
					{
						validEntries++;
						neighbourAvg += tempPixelData[kernel[i]].getColour_32_Bit().r;
					}
				}
				avg += (neighbourAvg / validEntries) * 0.5f;
				float finalColor = avg;
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
	if (windowSideVal == WindowSide::CENTER)
		normalViewStateUtility.zoomLevel += normalViewStateUtility.zoomLevel * 0.1f * yoffset;
	else if (windowSideVal == WindowSide::RIGHT)
		previewStateUtility.modelPreviewZoomLevel += 0.5f * yoffset;
}