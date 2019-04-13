#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <queue>
#include <GL\glew.h>
#include <GLFW/glfw3.h>
#include <GLM\gtc\quaternion.hpp>
#include <GLM\gtx\quaternion.hpp>

#include "ImGui\imgui.h"
#include "ImGui\imgui_impl_glfw.h"
#include "ImGui\imgui_impl_opengl2.h"

#include "GLutil.h"
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
#include "PreferencesHandler.h"
#include "LayerManager.h"
#include "NoraFileHandler.h"
//TODO : * Done but not good enough *Implement mouse position record and draw to prevent cursor skipping ( probably need separate thread for drawing |completly async| )
//Possible cause : Input take over by IMGUI
//TODO : Add Uniform Buffers
//TODO : Add shadows and an optional plane
//TODO : Mouse control when preview maximize panel opens
//TODO : Look into converting normal map to heightmap for editing purposes
//TODO : Control directional light direction through 3D hemisphere sun object in preview screen
//TODO : Reset view should make non 1:1 images fit in screen
//TODO : Convert text to icon for most buttons
//TODO : Fix memory error while using custom brush texture and exit the application
//TODO : Custom shader support for preview
//TODO : Better lighting options
//TODO : Moving the panel anywhere in the window and not zoom level effcted

enum class LoadingOption
{
	MODEL, TEXTURE, NONE
};

#pragma region STRING_CONSTANTS
const std::string VERSION_NAME = "v1.4 Beta";
const std::string FONTS_PATH = "Resources\\Fonts\\";
const std::string THEMES_PATH = "Resources\\Themes\\";
const std::string TEXTURES_PATH = "Resources\\Textures\\";
const std::string MATCAP_TEXTURES_PATH = "Resources\\Textures\\Matcaps\\";
const std::string CUBEMAP_TEXTURES_PATH = "Resources\\Textures\\Cubemaps\\";
const std::string BRUSH_TEXTURES_PATH = "Resources\\Brushes\\";
const std::string UI_TEXTURES_PATH = "Resources\\Textures\\UI\\";
const std::string SHADERS_PATH = "Resources\\Shaders\\";
const std::string PRIMITIVE_MODELS_PATH = "Resources\\3D Models\\Primitives\\";
const std::string COMPLEX_MODELS_PATH = "Resources\\3D Models\\Complex\\";
const std::string CUBE_MODEL_PATH = PRIMITIVE_MODELS_PATH + "Cube.fbx";
const std::string CYLINDER_MODEL_PATH = PRIMITIVE_MODELS_PATH + "Cylinder.fbx";
const std::string SPHERE_MODEL_PATH = PRIMITIVE_MODELS_PATH + "Sphere.fbx";
const std::string TORUS_MODEL_PATH = PRIMITIVE_MODELS_PATH + "Torus.fbx";
const std::string PLANE_MODEL_PATH = PRIMITIVE_MODELS_PATH + "Plane.fbx";
#pragma endregion

#pragma region FUNCTION_DECLARATIONS
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) noexcept;
void SetPixelValues(TextureData& texData, int startX, int width, int startY, int height, double xpos, double ypos);
void SetPixelValuesWithBrushTexture(TextureData& inputTexData, TextureData& brushTexture, int startX, int endX, int startY, int endY, double xpos, double ypos);
void SetBluredPixelValues(TextureData& inputTexData, int startX, int width, int startY, int height, double xpos, double ypos);
void SaveNormalMapToFile(const std::string &locationStr, ImageFormat imageFormat);
inline void HandleMiddleMouseButtonInput(int state, glm::vec2 &prevMiddleMouseButtonCoord, double deltaTime, DrawingPanel &normalmapPanel);
inline void HandleLeftMouseButtonInput_NormalMapInteraction(int state, DrawingPanel &normalmapPanel, bool isBlurOn);
inline void DisplayWindowTopBar(unsigned int minimizeTexture, unsigned int restoreTexture, bool &isMaximized, unsigned int closeTexture);
inline void DisplayBottomBar(const ImGuiWindowFlags &window_flags);
inline void DisplaySideBar(const ImGuiWindowFlags &window_flags, DrawingPanel &frameDrawingPanel, char saveLocation[500], bool &shouldSaveNormalMap, bool &changeSize);
inline void DisplayPreview(const ImGuiWindowFlags &window_flags);
inline void DisplayLayerPanel(const ImGuiWindowFlags &window_flags);
inline void DisplayLightSettingsUserInterface();
inline void DisplayNormalSettingsUserInterface();
inline void DisplayBrushSettingsUserInterface(bool &isBlurOn);
inline void HandleKeyboardInput(double deltaTime, DrawingPanel &frameDrawingPanel, bool &isMaximized);
void SetStatesForSavingNormalMap()noexcept;
void SetupImGui();
#pragma endregion

WindowSystem windowSys;
MeshLoadingSystem::MeshLoader modelLoader;
ImFont* menuBarLargerText = NULL;
PreviewStateUtility previewStateUtility;
NormalViewStateUtility normalViewStateUtility;

FrameBufferSystem fbs;
FrameBufferSystem previewFbs;

BrushData brushData;
TextureData heightMapTexData;
TextureData albedoTexDataForPreview;
TextureData metalnessTexDataForPreview;
TextureData roughnessTexDataForPreview;
TextureData matcapTexDataForPreview;

ModelObject *modelPreviewObj = nullptr;
LoadingOption currentLoadingOption = LoadingOption::NONE;
FileExplorer* fileExplorer;
ThemeManager* themeManager;
ModalWindow modalWindow;
DrawingPanel normalmapPanel;
UndoRedoSystem undoRedoSystem;

std::string heightImageLoadLocation = "";
PreferenceInfo preferencesInfo;
LayerManager layerManager;

#pragma region UI TEXTURE IDS
unsigned int toggleFullscreenTexId, resetViewTexId, clearViewTexId, maximizePreviewTexId; // UI textureIds
unsigned int closeTextureId, restoreTextureId, minimizeTextureId, logoTextureId; // Window textureIds
unsigned int defaultWhiteTextureId;
#pragma endregion

bool canPerformPreviewWindowMouseOperations = false;
bool isPreviewWindowMaximized = false;
bool isPreviewPanelActive = true;
bool isUsingLayerOutput = false;

int main(void)
{
#pragma region Window Initialization
	windowSys.init("Nora Normal Map Editor " + VERSION_NAME, 1600, 800);
	if (glewInit() != GLEW_OK)
	{
		std::cout << "Open GL init error" << std::endl;
		return EXIT_FAILURE;
	}
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	GL::enableBlending();
	GL::enableFaceCulling();
	GL::setFaceCullingMode(FaceCullingMode::BACK_FACE_CULLING);

	SetupImGui();
	//Initalize the File Explorer singleton
	FileExplorer::init();
	fileExplorer = FileExplorer::instance;
	//Load user preferences
	PreferencesHandler::init("Resources\\Preference\\preference.npref");
	preferencesInfo = PreferencesHandler::readPreferences();
	//Allocate undo/redo memory based on user preferences
	undoRedoSystem.updateAllocation(glm::vec2(512, 512), 4, preferencesInfo.maxUndoCount);
	//Initialize the theme manager singleton
	ThemeManager::init();
	themeManager = ThemeManager::instance;

	windowSys.setFrameBufferResizeCallback(framebuffer_size_callback);
	windowSys.setScrollCallback(scroll_callback);
#pragma endregion

	modelPreviewObj = modelLoader.createModelFromFile(CUBE_MODEL_PATH); // Default loaded model in preview window
	const ModelObject* cubeForSkybox = modelLoader.createModelFromFile(CUBE_MODEL_PATH);
	const ModelObject* previewGrid = modelLoader.createModelFromFile(PLANE_MODEL_PATH);
	const ModelObject* previewPlane = modelLoader.createModelFromFile(PLANE_MODEL_PATH);

	normalmapPanel.init(1.0f, 1.0f);
	DrawingPanel frameDrawingPanel;
	frameDrawingPanel.init(1.0f, 1.0f);
	DrawingPanel previewFrameDrawingPanel;
	previewFrameDrawingPanel.init(1.0f, 1.0f);
	DrawingPanel brushPanel;
	brushPanel.init(1.0f, 1.0f);

	//Windowing related images
	closeTextureId = TextureManager::createTextureFromFile(UI_TEXTURES_PATH + "closeIcon.png");
	restoreTextureId = TextureManager::createTextureFromFile(UI_TEXTURES_PATH + "maxWinIcon.png");
	minimizeTextureId = TextureManager::createTextureFromFile(UI_TEXTURES_PATH + "toTrayIcon.png");
	logoTextureId = TextureManager::createTextureFromFile(UI_TEXTURES_PATH + "icon.png");

	toggleFullscreenTexId = TextureManager::createTextureFromFile(UI_TEXTURES_PATH + "toggleFullscreen.png");
	clearViewTexId = TextureManager::createTextureFromFile(UI_TEXTURES_PATH + "clearView.png");
	resetViewTexId = TextureManager::createTextureFromFile(UI_TEXTURES_PATH + "resetLocation.png");
	maximizePreviewTexId = TextureManager::createTextureFromFile(UI_TEXTURES_PATH + "maximizePreview.png");
	defaultWhiteTextureId = TextureManager::createTextureFromColour(ColourData(1, 1, 1, 1));

	std::vector<std::string> cubeMapImagePaths;
	cubeMapImagePaths.push_back(CUBEMAP_TEXTURES_PATH + "Sahara Desert Cubemap\\sahara_lf.tga");
	cubeMapImagePaths.push_back(CUBEMAP_TEXTURES_PATH + "Sahara Desert Cubemap\\sahara_rt.tga");
	cubeMapImagePaths.push_back(CUBEMAP_TEXTURES_PATH + "Sahara Desert Cubemap\\sahara_dn.tga");
	cubeMapImagePaths.push_back(CUBEMAP_TEXTURES_PATH + "Sahara Desert Cubemap\\sahara_up.tga");
	cubeMapImagePaths.push_back(CUBEMAP_TEXTURES_PATH + "Sahara Desert Cubemap\\sahara_ft.tga");
	cubeMapImagePaths.push_back(CUBEMAP_TEXTURES_PATH + "Sahara Desert Cubemap\\sahara_bk.tga");

	unsigned int cubeMapTextureId = TextureManager::createCubemapFromFile(cubeMapImagePaths);
	albedoTexDataForPreview.SetTexId(TextureManager::createTextureFromFile(TEXTURES_PATH + "wall diffuse.png"));
	roughnessTexDataForPreview.SetTexId(TextureManager::createTextureFromFile(TEXTURES_PATH + "wall specular.png"));
	metalnessTexDataForPreview.SetTexId(defaultWhiteTextureId);
	matcapTexDataForPreview.SetTexId(TextureManager::createTextureFromFile(MATCAP_TEXTURES_PATH + "chrome.png"));

	heightImageLoadLocation = TEXTURES_PATH + "wall height.png";
	TextureManager::getTextureDataFromFile(heightImageLoadLocation, heightMapTexData);
	heightMapTexData.SetTexId(TextureManager::createTextureFromData(heightMapTexData));
	undoRedoSystem.record(heightMapTexData.getTextureData());
	normalmapPanel.setTextureID(heightMapTexData.GetTexId());

	Camera camera;
	camera.init(windowSys.getWindowRes().x, windowSys.getWindowRes().y);

#pragma region SHADER PROGRAMS
	ShaderProgram normalmapShader;
	normalmapShader.compileShaders(SHADERS_PATH + "normalPanel.vs", SHADERS_PATH + "normalPanel.fs");
	normalmapShader.linkShaders();

	ShaderProgram modelViewShader;
	modelViewShader.compileShaders(SHADERS_PATH + "modelView.vs", SHADERS_PATH + "modelView.fs");
	modelViewShader.linkShaders();

	ShaderProgram modelAttribViewShader;
	modelAttribViewShader.compileShaders(SHADERS_PATH + "modelAttribsDisplay.vs", SHADERS_PATH + "modelAttribsDisplay.fs", SHADERS_PATH + "modelAttribsDisplay.gs");
	modelAttribViewShader.linkShaders();

	ShaderProgram frameShader;
	frameShader.compileShaders(SHADERS_PATH + "normalPanel.vs", SHADERS_PATH + "frameBuffer.fs");
	frameShader.linkShaders();

	ShaderProgram brushPreviewShader;
	brushPreviewShader.compileShaders(SHADERS_PATH + "normalPanel.vs", SHADERS_PATH + "brushPreview.fs");
	brushPreviewShader.linkShaders();

	ShaderProgram gridLineShader;
	gridLineShader.compileShaders(SHADERS_PATH + "gridLines.vs", SHADERS_PATH + "gridLines.fs");
	gridLineShader.linkShaders();
#pragma endregion

#pragma region SHADER UNIFORM IDS
	//Normal map uniforms
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
	int textureOneIndexUniform = normalmapShader.getUniformLocation("textureOne");
	int textureTwoIndexUniform = normalmapShader.getUniformLocation("textureTwo");
	int useNormalInputUniform = normalmapShader.getUniformLocation("_UseNormalInput");
	int normalBlendingMethod = normalmapShader.getUniformLocation("_NormalBlendingMethod");

	//Brush uniforms
	int brushPreviewModelUniform = brushPreviewShader.getUniformLocation("model");
	int brushPreviewTextureUniform = brushPreviewShader.getUniformLocation("_BrushTexture");
	int brushPreviewOffsetUniform = brushPreviewShader.getUniformLocation("_BrushOffset");
	int brushPreviewStrengthUniform = brushPreviewShader.getUniformLocation("_BrushStrength");
	int brushPreviewColourUniform = brushPreviewShader.getUniformLocation("_BrushColour");
	int brushPreviewUseTextureUniform = brushPreviewShader.getUniformLocation("_UseTexture");

	//Model preview uniforms
	int modelPreviewModelUniform = modelViewShader.getUniformLocation("model");
	int modelPreviewViewUniform = modelViewShader.getUniformLocation("view");
	int modelPreviewProjectionUniform = modelViewShader.getUniformLocation("projection");
	int modelCameraPos = modelViewShader.getUniformLocation("_CameraPosition");
	int modelWidthUniform = modelViewShader.getUniformLocation("_HeightmapDimX");
	int modelHeightUniform = modelViewShader.getUniformLocation("_HeightmapDimY");
	int modelNormalMapModeUniform = modelViewShader.getUniformLocation("_normalMapModeOn");
	int modelNormalMapStrengthUniform = modelViewShader.getUniformLocation("_HeightmapStrength");
	int modelLightIntensityUniform = modelViewShader.getUniformLocation("_LightIntensity");
	int modelRoughnessUniform = modelViewShader.getUniformLocation("_Roughness");
	int modelMetalnessUniform = modelViewShader.getUniformLocation("_Metalness");
	int modelLightPositionUniform = modelViewShader.getUniformLocation("lightPos");
	int modelLightColourUniform = modelViewShader.getUniformLocation("lightColour");
	int modelDiffuseColourUniform = modelViewShader.getUniformLocation("diffuseColour");
	int modelHeightMapTextureUniform = modelViewShader.getUniformLocation("heightmapTexture");
	int modelAlbedoMapTextureUniform = modelViewShader.getUniformLocation("albedomapTexture");
	int modelMetalnessMapTextureUniform = modelViewShader.getUniformLocation("metalnessmapTexture");
	int modelRoughnessMapTextureUniform = modelViewShader.getUniformLocation("roughnessmapTexture");
	int modelMatcapTextureUniform = modelViewShader.getUniformLocation("mapcapTexture");
	int modelCubeMapTextureUniform = modelViewShader.getUniformLocation("skybox");
	int modelMethodIndexUniform = modelViewShader.getUniformLocation("_MethodIndex");
	int modelUseMatcapUniform = modelViewShader.getUniformLocation("_Use_Matcap");

	//Model attributes uniforms
	int modelAttributesShowNormalsUniform = modelAttribViewShader.getUniformLocation("_ShowNormals");
	int modelAttributesNormalLengthUniform = modelAttribViewShader.getUniformLocation("_NormalsLength");

	//Gridlines uniforms
	int gridLineModelMatrixUniform = gridLineShader.getUniformLocation("model");
	int gridLineViewMatrixUniform = gridLineShader.getUniformLocation("view");
	int gridLineProjectionMatrixUniform = gridLineShader.getUniformLocation("projection");
#pragma endregion

	bool isMaximized = false;
	bool isBlurOn = false;

	layerManager.init(windowSys.getWindowRes(), glm::vec2(preferencesInfo.maxWidthRes, preferencesInfo.maxHeightRes));
	layerManager.addLayer(heightMapTexData.GetTexId(), LayerType::HEIGHT_MAP);

	fbs.init(windowSys.getWindowRes(), glm::vec2(preferencesInfo.maxWidthRes, preferencesInfo.maxHeightRes));
	previewFbs.init(windowSys.getWindowRes(), glm::vec2(1920, 1920));

	FrameBufferSystem layersNormalOutputFbs;
	layersNormalOutputFbs.init(windowSys.getWindowRes(), glm::vec2(preferencesInfo.maxWidthRes, preferencesInfo.maxHeightRes));

	glm::vec2 prevMouseCoord = glm::vec2(-10, -10);
	glm::vec2 prevMiddleMouseButtonCoord = glm::vec2(-10, -10);

	bool shouldSaveNormalMap = false;
	bool changeSize = false;

	//std::thread applyPanelChangeThread(ApplyChangesToPanel);
	double initTime = glfwGetTime();
	while (!windowSys.isWindowClosing())
	{
		const double deltaTime = glfwGetTime() - initTime;
		initTime = glfwGetTime();
		if (shouldSaveNormalMap)
			SetStatesForSavingNormalMap();
		static glm::vec2 initPos = glm::vec2(-1000, -1000);
		static WindowSide windowSideAtInitPos = WindowSide::NONE;

		const glm::vec2 curMouseCoord = windowSys.getCursorPos();
		HandleKeyboardInput(deltaTime, frameDrawingPanel, isMaximized);

		//---- Making sure the dimensions do not change for drawing panel ----//
		const float aspectRatio = windowSys.getAspectRatio();
		glm::vec2 aspectRatioHolder;
		if (windowSys.getWindowRes().x < windowSys.getWindowRes().y)
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
		frameDrawingPanel.getTransform()->update();
		const int leftMouseButtonState = glfwGetMouseButton((GLFWwindow*)windowSys.getWindow(), GLFW_MOUSE_BUTTON_LEFT);
		const int middleMouseButtonState = glfwGetMouseButton((GLFWwindow*)windowSys.getWindow(), GLFW_MOUSE_BUTTON_MIDDLE);

		//Check if the preview modal window is open and only then perform drawing operations
		if (!isPreviewWindowMaximized)
		{
			HandleMiddleMouseButtonInput(middleMouseButtonState, prevMiddleMouseButtonCoord, deltaTime, frameDrawingPanel);
			HandleLeftMouseButtonInput_NormalMapInteraction(leftMouseButtonState, frameDrawingPanel, isBlurOn);
		}

		heightMapTexData.updateTexture();

		glViewport(0, 0, windowSys.getWindowRes().x, windowSys.getWindowRes().y);
		GL::setClearColour(0.9f, 0.5f, 0.2f);
		GL::enableDepthTest();
		GL::setDepthTestMode(DepthTestMode::DEPTH_LESS);
		normalmapPanel.getTransform()->update();
		normalmapShader.use();
		//---- Applying Normal Map Shader Uniforms---//
		normalmapShader.applyShaderUniformMatrix(normalPanelModelMatrixUniform, glm::mat4(1));
		normalmapShader.applyShaderFloat(strengthValueUniform, normalViewStateUtility.normalMapStrength);
		normalmapShader.applyShaderFloat(specularityUniform, normalViewStateUtility.specularity);
		normalmapShader.applyShaderFloat(specularityStrengthUniform, normalViewStateUtility.specularityStrength);
		normalmapShader.applyShaderFloat(lightIntensityUniform, normalViewStateUtility.lightIntensity);
		normalmapShader.applyShaderVector3(lightDirectionUniform, normalViewStateUtility.getNormalizedLightDir());
		normalmapShader.applyShaderFloat(widthUniform, heightMapTexData.getRes().x);
		normalmapShader.applyShaderFloat(heightUniform, heightMapTexData.getRes().y);
		normalmapShader.applyShaderBool(flipXYdirUniform, normalViewStateUtility.flipX_Ydir);
		normalmapShader.applyShaderBool(RedChannelUniform, normalViewStateUtility.redChannelActive);
		normalmapShader.applyShaderBool(GreenChannelUniform, normalViewStateUtility.greenChannelActive);
		normalmapShader.applyShaderBool(BlueChannelUniform, normalViewStateUtility.blueChannelActive);
		normalmapShader.applyShaderBool(methodIndexUniform, normalViewStateUtility.methodIndex);
		normalmapShader.applyShaderInt(textureOneIndexUniform, 0);
		normalmapShader.applyShaderInt(textureTwoIndexUniform, 1);
		normalmapShader.applyShaderInt(useNormalInputUniform, 3);
		//---- Draw each of the layers to a frame buffer----//
		for (int layerIndex = 0; layerIndex < layerManager.getLayerCount(); layerIndex++)
		{
			layerManager.bindFrameBuffer(layerIndex);
			GL::clear(FrameBufferAttachment::COLOUR_AND_DEPTH_BUFFER);

			normalmapShader.applyShaderFloat(strengthValueUniform, (layerIndex == 0) ? normalViewStateUtility.normalMapStrength : layerManager.getLayerStrength(layerIndex));
			normalmapShader.applyShaderInt(normalMapModeOnUniform, (layerManager.getLayerType(layerIndex) == LayerType::HEIGHT_MAP) ? 1 : 3);
			if (layerIndex == 0 && !isUsingLayerOutput)
				normalmapShader.applyShaderInt(normalMapModeOnUniform, normalViewStateUtility.mapDrawViewMode);
			normalmapPanel.setTextureID(layerManager.getInputTexId(layerIndex), false);
			normalmapPanel.draw();
		}

		if (shouldSaveNormalMap)
			SetStatesForSavingNormalMap();
		//---- Bind main frame buffer and set output to cumulative blending ----//
		GL::disableDepthTest();
		fbs.bindFrameBuffer();
		GL::clear(FrameBufferAttachment::COLOUR_AND_DEPTH_BUFFER);

		normalmapPanel.setTextureID(layerManager.getColourTexture(0), false);
		normalmapShader.applyShaderInt(useNormalInputUniform, 2);
		normalmapShader.applyShaderInt(normalBlendingMethod, (int)layerManager.getNormalBlendMethod(0));
		normalmapShader.applyShaderInt(normalMapModeOnUniform, 0);
		normalmapPanel.draw();

		if (isUsingLayerOutput)
		{
			normalmapShader.applyShaderInt(useNormalInputUniform, 1);

			if (layerManager.getLayerCount() >= 1)
			{
				for (int i = 1; i < layerManager.getLayerCount(); i++)
				{
					if (!layerManager.isLayerActive(i))
						continue;
					normalmapShader.applyShaderInt(normalBlendingMethod, (int)layerManager.getNormalBlendMethod(i));
					normalmapPanel.setTextureID(fbs.getColourTexture(), false);
					normalmapPanel.draw(layerManager.getColourTexture(i));
				}
			}

			//Copy contant from one frame buffer to another
			FrameBufferSystem::blit(fbs, layersNormalOutputFbs, windowSys.getMaxWindowRes());

			normalmapShader.applyShaderInt(useNormalInputUniform, 2);
			normalmapShader.applyShaderInt(normalMapModeOnUniform, normalViewStateUtility.mapDrawViewMode);
			normalmapPanel.draw();
		}

		FrameBufferSystem::bindDefaultFrameBuffer();
		GL::setClearColour(0.1f, 0.1f, 0.1f);
		GL::clear(FrameBufferAttachment::COLOUR_AND_DEPTH_BUFFER);

		static char saveLocation[500] = { '\0' };
		if (saveLocation[0] == '\0')
			std::memcpy(saveLocation, &preferencesInfo.defaultExportPath[0], preferencesInfo.defaultExportPath.size());

		if (shouldSaveNormalMap)
		{
			ImageFormat imageFormat = ImageFormat::BMP;
			//Image validation stage start
			std::string path(saveLocation);
			std::string fileExt = fileExplorer->getFileExtension(saveLocation);
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
				//fbs.updateTextureDimensions(windowSys.getWindowRes().x, windowSys.getWindowRes().y);
				continue;
			}
			//Image validation stage over
			SaveNormalMapToFile(saveLocation, imageFormat);
			shouldSaveNormalMap = false;
			modalWindow.setModalDialog("INFO", "Image exported to : " + path + "\nResolution : " +
				std::to_string((int)heightMapTexData.getRes().x) + "x" + std::to_string((int)heightMapTexData.getRes().y));
			continue;
		}

		if (windowSys.isKeyPressedDown(GLFW_KEY_F10))
		{
			shouldSaveNormalMap = true;
			changeSize = true;
		}

		// Draw the frame by using the frame buffer texture
		frameShader.use();
		frameShader.applyShaderUniformMatrix(frameModelMatrixUniform, frameDrawingPanel.getTransform()->getMatrix());
		frameDrawingPanel.setTextureID(fbs.getColourTexture());
		frameDrawingPanel.draw();

		// Set up the preview frame buffer and then render the 3d model
		previewFbs.bindFrameBuffer();

		GL::clear(FrameBufferAttachment::COLOUR_AND_DEPTH_BUFFER);
		GL::enableDepthTest();
		GL::setDepthTestMode(DepthTestMode::DEPTH_LESS);

		static float circleAround = 2.5f;
		static float yAxis = -2.0f;
		glm::vec3 cameraPosition;
		static glm::vec2 prevMcord;
		glm::vec2 offset = (prevMcord - windowSys.getCursorPos());

		if (leftMouseButtonState == GLFW_PRESS && glm::length(offset) > 0.0f && canPerformPreviewWindowMouseOperations)
		{
			circleAround += offset.x * 0.01f;
			yAxis += offset.y * 0.01f;
		}
		yAxis = glm::clamp(yAxis, -100.0f, 100.0f);
		cameraPosition.x = glm::sin(circleAround) * previewStateUtility.modelPreviewZoomLevel;
		cameraPosition.z = glm::cos(circleAround) * previewStateUtility.modelPreviewZoomLevel;
		cameraPosition.y = yAxis;

		if (glm::length(cameraPosition) > previewStateUtility.modelPreviewZoomLevel)
			cameraPosition = -glm::normalize(cameraPosition) * previewStateUtility.modelPreviewZoomLevel;
		prevMcord = windowSys.getCursorPos();

		// Set up preview model uniforms
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
		modelViewShader.applyShaderFloat(modelRoughnessUniform, previewStateUtility.roughness);
		modelViewShader.applyShaderFloat(modelMetalnessUniform, previewStateUtility.metalness);
		glm::vec3 lightPos;
		lightPos.x = sin(previewStateUtility.lightLocation.x) * 3;
		lightPos.z = cos(previewStateUtility.lightLocation.x) * 3;
		lightPos.y = previewStateUtility.lightLocation.y;
		modelViewShader.applyShaderVector3(modelLightPositionUniform, lightPos);
		modelViewShader.applyShaderInt(modelHeightMapTextureUniform, 0);
		modelViewShader.applyShaderInt(modelAlbedoMapTextureUniform, 1);
		modelViewShader.applyShaderInt(modelMetalnessMapTextureUniform, 2);
		modelViewShader.applyShaderInt(modelRoughnessMapTextureUniform, 3);
		modelViewShader.applyShaderInt(modelMatcapTextureUniform, 4);
		modelViewShader.applyShaderInt(modelCubeMapTextureUniform, 5);

		modelViewShader.applyShaderVector3(modelDiffuseColourUniform, previewStateUtility.diffuseColour);
		modelViewShader.applyShaderVector3(modelLightColourUniform, previewStateUtility.lightColour);
		modelViewShader.applyShaderInt(modelMethodIndexUniform, (isUsingLayerOutput) ? 2 : normalViewStateUtility.methodIndex);

		modelViewShader.applyShaderBool(modelUseMatcapUniform, previewStateUtility.useMatcap);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, (isUsingLayerOutput) ? layersNormalOutputFbs.getColourTexture() : heightMapTexData.GetTexId());
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, albedoTexDataForPreview.GetTexId());
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, metalnessTexDataForPreview.GetTexId());
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, roughnessTexDataForPreview.GetTexId());
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, matcapTexDataForPreview.GetTexId());
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTextureId);
		if (modelPreviewObj != nullptr)
			modelPreviewObj->draw();

		modelAttribViewShader.use();
		modelAttribViewShader.applyShaderUniformMatrix(modelPreviewModelUniform, glm::mat4());
		modelAttribViewShader.applyShaderUniformMatrix(modelPreviewViewUniform, glm::lookAt(cameraPosition, glm::vec3(0), glm::vec3(0, 1, 0)));
		modelAttribViewShader.applyShaderUniformMatrix(modelPreviewProjectionUniform, glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f));
		modelAttribViewShader.applyShaderBool(modelAttributesShowNormalsUniform, previewStateUtility.showNormals);
		modelAttribViewShader.applyShaderFloat(modelAttributesNormalLengthUniform, previewStateUtility.normDisplayLineLength);
		if (modelPreviewObj != nullptr)
			modelPreviewObj->draw();
		glActiveTexture(GL_TEXTURE0);

#pragma region GRID SETUP & RENDER
		// Set up preview shader uniforms
		gridLineShader.use();
		gridLineShader.applyShaderUniformMatrix(gridLineModelMatrixUniform, glm::scale(glm::mat4(), glm::vec3(100, 0, 100)));
		gridLineShader.applyShaderUniformMatrix(gridLineViewMatrixUniform, glm::lookAt(cameraPosition, glm::vec3(0), glm::vec3(0, 1, 0)));
		gridLineShader.applyShaderUniformMatrix(gridLineProjectionMatrixUniform, glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f));
		previewGrid->draw();
#pragma endregion
		// Set up the default framebuffer
		FrameBufferSystem::bindDefaultFrameBuffer();
#pragma region  SETUP & RENDER BRUSH DATA
		if (windowSys.getWindowRes().x < windowSys.getWindowRes().y)
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
		brushPanel.getTransform()->setPosition(((curMouseCoord.x / windowSys.getWindowRes().x)*2.0f) - 1.0f,
			-(((curMouseCoord.y / windowSys.getWindowRes().y)*2.0f) - 1.0f));
		brushPanel.getTransform()->update();
		brushPreviewShader.applyShaderFloat(brushPreviewStrengthUniform, brushData.brushStrength);
		brushPreviewShader.applyShaderFloat(brushPreviewOffsetUniform, glm::pow(brushData.brushOffset, 2) * 10.0f);
		brushPreviewShader.applyShaderVector3(brushPreviewColourUniform, (brushData.heightMapPositiveDir ? glm::vec3(1) : glm::vec3(0)));
		brushPreviewShader.applyShaderBool(brushPreviewUseTextureUniform, !brushData.hasBrushTexture());
		brushPreviewShader.applyShaderUniformMatrix(brushPreviewModelUniform, brushPanel.getTransform()->getMatrix());

		brushPanel.setTextureID(brushData.textureData.GetTexId());
		brushPanel.draw();
#pragma endregion
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

		if (isPreviewPanelActive)
		{
			//________Preview Display_______
			DisplayPreview(window_flags);
		}
		else
		{
			//________Layers Display________
			DisplayLayerPanel(window_flags);
		}
		fileExplorer->display();
		modalWindow.display();

		//static bool ff = true;
		//ImGui::ShowDemoWindow(&ff);

		ImGui::Render();

		glBindVertexArray(0);
		glUseProgram(0);
		ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
		windowSys.updateWindow();

	}
	//applyPanelChangeThread.join();
	brushData.textureData.clearRawData();

	delete modelPreviewObj;
	delete cubeForSkybox;
	delete previewGrid;

	fileExplorer->shutDown();
	themeManager->shutDown();
	ImGui_ImplOpenGL2_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	windowSys.destroy();
	return 0;
}
inline void DisplaySideBar(const ImGuiWindowFlags &window_flags, DrawingPanel &frameDrawingPanel, char saveLocation[500], bool &shouldSaveNormalMap, bool &changeSize)
{
	bool open = true;
	ImGui::SetNextWindowPos(ImVec2(windowSys.getWindowRes().x - 5, 42), ImGuiSetCond_Always);
	ImGui::SetNextWindowSize(ImVec2(10, windowSys.getWindowRes().y - 67), ImGuiSetCond_Always);

	ImGui::SetNextWindowPos(ImVec2(0, 42), ImGuiSetCond_Always);
	ImGui::SetNextWindowSize(ImVec2(glm::clamp(windowSys.getWindowRes().x * 0.15f, 280.0f, 600.0f), windowSys.getWindowRes().y - 77), ImGuiSetCond_Always);
	ImGui::Begin("Settings", &open, window_flags);
	ImGui::PushStyleColor(ImGuiCol_Button, themeManager->SecondaryColour);
	float buttonWidth = ImGui::GetContentRegionAvailWidth() / 3.5f;
	if (ImGui::ImageButton((ImTextureID)toggleFullscreenTexId, ImVec2(buttonWidth, 40), ImVec2(-0.4f, 1.0f), ImVec2(1.4f, 0.0f), -1, ImVec4(0, 0, 0, 0), themeManager->AccentColour2))
	{
		if (!windowSys.getIfFullscreen())
			windowSys.setFullscreen(true);
		else
			windowSys.setFullscreen(false);
	}
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Toggle Fullscreen (Ctrl + T)");
	ImGui::SameLine();
	if (ImGui::ImageButton((ImTextureID)clearViewTexId, ImVec2(buttonWidth, 40), ImVec2(-0.4f, 1.0f), ImVec2(1.4f, 0.0f), -1, ImVec4(0, 0, 0, 0), themeManager->AccentColour2))
	{
		std::memset(heightMapTexData.getTextureData(), 255, heightMapTexData.getRes().y * heightMapTexData.getRes().x * heightMapTexData.getComponentCount());
		undoRedoSystem.record(heightMapTexData.getTextureData());
		heightMapTexData.setTextureDirty();
	}
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Clear the panel (Ctrl + Alt + V)");
	ImGui::SameLine();
	if (ImGui::ImageButton((ImTextureID)resetViewTexId, ImVec2(buttonWidth, 40), ImVec2(-0.4f, 0.0f), ImVec2(1.4f, 1.0f), -1, ImVec4(0, 0, 0, 0), themeManager->AccentColour2))
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
	ImGui::Spacing();
	static int item_type;
	ImGui::RadioButton("RAW DATA", &item_type, 0); ImGui::SameLine();
	ImGui::RadioButton("LAYER OUTPUT", &item_type, 1);
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 10));
	int modeButtonWidth = (int)(ImGui::GetContentRegionAvailWidth() / 3.0f);
	ImGui::Spacing();
	if (normalViewStateUtility.mapDrawViewMode == 3) ImGui::PushStyleColor(ImGuiCol_Button, themeManager->AccentColour1);
	else ImGui::PushStyleColor(ImGuiCol_Button, themeManager->SecondaryColour);

	isUsingLayerOutput = (item_type == 0) ? false : true;
	if (isUsingLayerOutput)
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
	if (ImGui::Button("Height", ImVec2(modeButtonWidth - 5, 40)))
	{
		normalViewStateUtility.mapDrawViewMode = (isUsingLayerOutput) ? normalViewStateUtility.mapDrawViewMode : 0;
	}
	if (isUsingLayerOutput)
		ImGui::PopStyleVar();
	ImGui::PopStyleColor();
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("(Ctrl + H)");

	ImGui::SameLine(0, 5);
	if (normalViewStateUtility.mapDrawViewMode == 1) ImGui::PushStyleColor(ImGuiCol_Button, themeManager->AccentColour1);
	else ImGui::PushStyleColor(ImGuiCol_Button, themeManager->SecondaryColour);
	if (ImGui::Button("Normal", ImVec2(modeButtonWidth - 5, 40))) { normalViewStateUtility.mapDrawViewMode = 1; }
	ImGui::PopStyleColor();
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("(Ctrl + J)");

	ImGui::SameLine(0, 5);
	if (normalViewStateUtility.mapDrawViewMode == 2) ImGui::PushStyleColor(ImGuiCol_Button, themeManager->AccentColour1);
	else ImGui::PushStyleColor(ImGuiCol_Button, themeManager->SecondaryColour);
	if (ImGui::Button("Lighting", ImVec2(modeButtonWidth, 40))) { normalViewStateUtility.mapDrawViewMode = 2; }
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("(Ctrl + K)");
	ImGui::PopStyleColor();
	ImGui::PopStyleVar();
}
void DisplayBottomBar(const ImGuiWindowFlags &window_flags)
{
	ImGui::SetNextWindowPos(ImVec2(0, windowSys.getWindowRes().y - 35), ImGuiSetCond_Always);
	ImGui::SetNextWindowSize(ImVec2(windowSys.getWindowRes().x, 50), ImGuiSetCond_Always);
	bool open = true;
	ImGui::Begin("Bottom_Bar", &open, window_flags);

	float totalWidth = ImGui::GetContentRegionAvailWidth();

	std::string imageDataText(heightImageLoadLocation + " : ");
	imageDataText += std::to_string((int)heightMapTexData.getRes().x) + "x" + std::to_string((int)heightMapTexData.getRes().y);
	ImGui::Text(imageDataText.c_str()); ImGui::SameLine();

	float diffWidth = totalWidth - ImGui::GetContentRegionAvailWidth();

	ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvailWidth() * 0.5f - diffWidth * 0.5f, 0)); ImGui::SameLine();
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
		const int count = glm::abs(currentSection - prevSection);
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
	ImGui_ImplGlfw_InitForOpenGL((GLFWwindow*)windowSys.getWindow(), true);
	ImGui_ImplOpenGL2_Init();

	ImFont* font = io.Fonts->AddFontFromFileTTF("Resources\\Fonts\\Roboto-Medium.ttf", 16.0f);
	menuBarLargerText = io.Fonts->AddFontFromFileTTF("Resources\\Fonts\\Roboto-Medium.ttf", 18.0f);

	modalWindow.setTitleFont(menuBarLargerText);

	IM_ASSERT(font != NULL);
	IM_ASSERT(menuBarLargerText != NULL);
}
void SetStatesForSavingNormalMap() noexcept
{
	glViewport(0, 0, heightMapTexData.getRes().x, heightMapTexData.getRes().y);
	fbs.updateTextureDimensions(heightMapTexData.getRes().x, heightMapTexData.getRes().y);
}
void HandleKeyboardInput(double deltaTime, DrawingPanel &frameDrawingPanel, bool &isMaximized)
{
	//Normal map strength and zoom controls for normal map
	if (windowSys.isKeyPressed(GLFW_KEY_LEFT) && windowSys.isKeyPressed(GLFW_KEY_LEFT_ALT) && !windowSys.isKeyPressed(GLFW_KEY_LEFT_SHIFT) && !windowSys.isKeyPressed(GLFW_KEY_LEFT_CONTROL))
		normalViewStateUtility.normalMapStrength -= 0.05f;
	if (windowSys.isKeyPressed(GLFW_KEY_RIGHT) && windowSys.isKeyPressed(GLFW_KEY_LEFT_ALT) && !windowSys.isKeyPressed(GLFW_KEY_LEFT_SHIFT) && !windowSys.isKeyPressed(GLFW_KEY_LEFT_CONTROL))
		normalViewStateUtility.normalMapStrength += 0.05f;
	if (windowSys.isKeyPressed(GLFW_KEY_W))
		normalViewStateUtility.zoomLevel += normalViewStateUtility.zoomLevel * 1.5f * deltaTime;
	if (windowSys.isKeyPressed(GLFW_KEY_S))
		normalViewStateUtility.zoomLevel -= normalViewStateUtility.zoomLevel * 1.5f * deltaTime;
	normalViewStateUtility.zoomLevel = glm::clamp(normalViewStateUtility.zoomLevel, 0.1f, 5.0f);

	//Normal map channel toggles
	if (windowSys.isKeyPressedDown(GLFW_KEY_R) && windowSys.isKeyPressed(GLFW_KEY_LEFT_SHIFT))
		normalViewStateUtility.redChannelActive = !normalViewStateUtility.redChannelActive;
	if (windowSys.isKeyPressedDown(GLFW_KEY_G) && windowSys.isKeyPressed(GLFW_KEY_LEFT_SHIFT))
		normalViewStateUtility.greenChannelActive = !normalViewStateUtility.greenChannelActive;
	if (windowSys.isKeyPressedDown(GLFW_KEY_B) && windowSys.isKeyPressed(GLFW_KEY_LEFT_SHIFT))
		normalViewStateUtility.blueChannelActive = !normalViewStateUtility.blueChannelActive;

	//Set normal map view mode in editor
	if (windowSys.isKeyPressed(GLFW_KEY_H) && windowSys.isKeyPressed(GLFW_KEY_LEFT_CONTROL))
		normalViewStateUtility.mapDrawViewMode = 3;
	if (windowSys.isKeyPressed(GLFW_KEY_J) && windowSys.isKeyPressed(GLFW_KEY_LEFT_CONTROL))
		normalViewStateUtility.mapDrawViewMode = 1;
	if (windowSys.isKeyPressed(GLFW_KEY_K) && windowSys.isKeyPressed(GLFW_KEY_LEFT_CONTROL))
		normalViewStateUtility.mapDrawViewMode = 2;

	//Set normal map view mode in model preview
	if (windowSys.isKeyPressed(GLFW_KEY_H) && windowSys.isKeyPressed(GLFW_KEY_LEFT_ALT))
		previewStateUtility.modelViewMode = 3;
	if (windowSys.isKeyPressed(GLFW_KEY_J) && windowSys.isKeyPressed(GLFW_KEY_LEFT_ALT))
		previewStateUtility.modelViewMode = 1;
	if (windowSys.isKeyPressed(GLFW_KEY_K) && windowSys.isKeyPressed(GLFW_KEY_LEFT_ALT))
		previewStateUtility.modelViewMode = 2;
	if (windowSys.isKeyPressed(GLFW_KEY_L) && windowSys.isKeyPressed(GLFW_KEY_LEFT_ALT))
		previewStateUtility.modelViewMode = 4;

	//Normal map movement
	if (windowSys.isKeyPressed(GLFW_KEY_LEFT) && !windowSys.isKeyPressed(GLFW_KEY_LEFT_SHIFT) && !windowSys.isKeyPressed(GLFW_KEY_LEFT_ALT) && !windowSys.isKeyPressed(GLFW_KEY_LEFT_CONTROL))
		frameDrawingPanel.getTransform()->translate(-1.0f * deltaTime, 0.0f);
	if (windowSys.isKeyPressed(GLFW_KEY_RIGHT) && !windowSys.isKeyPressed(GLFW_KEY_LEFT_SHIFT) && !windowSys.isKeyPressed(GLFW_KEY_LEFT_ALT) && !windowSys.isKeyPressed(GLFW_KEY_LEFT_CONTROL))
		frameDrawingPanel.getTransform()->translate(1.0f * deltaTime, 0);
	if (windowSys.isKeyPressed(GLFW_KEY_UP) && !windowSys.isKeyPressed(GLFW_KEY_LEFT_SHIFT) && !windowSys.isKeyPressed(GLFW_KEY_LEFT_ALT && !windowSys.isKeyPressed(GLFW_KEY_LEFT_CONTROL)))
		frameDrawingPanel.getTransform()->translate(0.0f, 1.0f * deltaTime);
	if (windowSys.isKeyPressed(GLFW_KEY_DOWN) && !windowSys.isKeyPressed(GLFW_KEY_LEFT_SHIFT) && !windowSys.isKeyPressed(GLFW_KEY_LEFT_ALT && !windowSys.isKeyPressed(GLFW_KEY_LEFT_CONTROL)))
		frameDrawingPanel.getTransform()->translate(0.0f, -1.0f * deltaTime);

	//Undo
	if (windowSys.isKeyPressedDown(GLFW_KEY_Z) && windowSys.isKeyPressed(GLFW_KEY_LEFT_CONTROL))
	{
		heightMapTexData.updateTextureData(undoRedoSystem.retrieve());
		heightMapTexData.updateTexture();
		if (undoRedoSystem.getCurrentSectionPosition() == 0)
			undoRedoSystem.record(heightMapTexData.getTextureData());
	}
	//Redo
	if (windowSys.isKeyPressedDown(GLFW_KEY_Y) && windowSys.isKeyPressed(GLFW_KEY_LEFT_CONTROL))
	{
		heightMapTexData.updateTextureData(undoRedoSystem.retrieve(false));
		heightMapTexData.updateTexture();
		if (undoRedoSystem.getCurrentSectionPosition() == 0)
			undoRedoSystem.record(heightMapTexData.getTextureData());
	}
	//Open new image
	if (windowSys.isKeyPressedDown(GLFW_KEY_O) && windowSys.isKeyPressed(GLFW_KEY_LEFT_CONTROL))
	{
		currentLoadingOption = LoadingOption::TEXTURE;
		fileExplorer->displayDialog(FileType::IMAGE, [&](std::string str)
		{
			if (currentLoadingOption == LoadingOption::TEXTURE)
			{
				TextureManager::getTextureDataFromFile(str, heightMapTexData);
				heightMapTexData.SetTexId(TextureManager::createTextureFromData(heightMapTexData));
				heightMapTexData.setTextureDirty();
				normalmapPanel.setTextureID(heightMapTexData.GetTexId());

				undoRedoSystem.updateAllocation(heightMapTexData.getRes(), heightMapTexData.getComponentCount(), preferencesInfo.maxUndoCount);
				undoRedoSystem.record(heightMapTexData.getTextureData());
			}
		});
	}

	//Brush data
	if (windowSys.isKeyPressed(GLFW_KEY_UP) && windowSys.isKeyPressed(GLFW_KEY_LEFT_SHIFT) && !windowSys.isKeyPressed(GLFW_KEY_LEFT_CONTROL))
		brushData.brushScale += 0.1f;
	if (windowSys.isKeyPressed(GLFW_KEY_DOWN) && windowSys.isKeyPressed(GLFW_KEY_LEFT_SHIFT) && !windowSys.isKeyPressed(GLFW_KEY_LEFT_CONTROL))
		brushData.brushScale -= 0.1f;
	if (windowSys.isKeyPressed(GLFW_KEY_RIGHT) && windowSys.isKeyPressed(GLFW_KEY_LEFT_SHIFT) && !windowSys.isKeyPressed(GLFW_KEY_LEFT_CONTROL))
		brushData.brushOffset += 0.01f;
	if (windowSys.isKeyPressed(GLFW_KEY_LEFT) && windowSys.isKeyPressed(GLFW_KEY_LEFT_SHIFT) && !windowSys.isKeyPressed(GLFW_KEY_LEFT_CONTROL))
		brushData.brushOffset -= 0.01f;
	if (windowSys.isKeyPressed(GLFW_KEY_UP) && windowSys.isKeyPressed(GLFW_KEY_LEFT_SHIFT) && windowSys.isKeyPressed(GLFW_KEY_LEFT_CONTROL))
		brushData.brushStrength += 0.01f;
	if (windowSys.isKeyPressed(GLFW_KEY_DOWN) && windowSys.isKeyPressed(GLFW_KEY_LEFT_SHIFT) && windowSys.isKeyPressed(GLFW_KEY_LEFT_CONTROL))
		brushData.brushStrength -= 0.01f;
	brushData.brushOffset = glm::clamp(brushData.brushOffset, 0.01f, 1.0f);
	brushData.brushScale = glm::clamp(brushData.brushScale, 1.0f, heightMapTexData.getRes().y * 0.5f);
	brushData.brushStrength = glm::clamp(brushData.brushStrength, 0.0f, 1.0f);

	//Flip normal map x-y axis
	if (windowSys.isKeyPressedDown(GLFW_KEY_A) && windowSys.isKeyPressed(GLFW_KEY_LEFT_CONTROL))
		normalViewStateUtility.flipX_Ydir = !normalViewStateUtility.flipX_Ydir;

	//Window fullscreen toggle
	if (windowSys.isKeyPressedDown(GLFW_KEY_T) && windowSys.isKeyPressed(GLFW_KEY_LEFT_CONTROL))
	{
		if (!windowSys.getIfFullscreen())
			windowSys.setFullscreen(true);
		else
			windowSys.setFullscreen(false);
	}

	//Normal map panel reset/clear
	if (windowSys.isKeyPressedDown(GLFW_KEY_V) && windowSys.isKeyPressed(GLFW_KEY_LEFT_CONTROL))
	{
		//clear
		if (windowSys.isKeyPressed(GLFW_KEY_LEFT_ALT))
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
	if (windowSys.isKeyPressed(GLFW_KEY_F9))
	{
		windowSys.setWindowRes(1600, 800);
		isMaximized = false;
	}

	//Save .nora to file and read
	if (windowSys.isKeyPressed(GLFW_KEY_2) && windowSys.isKeyPressed(GLFW_KEY_LEFT_ALT))
	{
		NoraFileHandler::writeToDisk("Test.pic", heightMapTexData, layerManager);
	}
	if (windowSys.isKeyPressed(GLFW_KEY_3) && windowSys.isKeyPressed(GLFW_KEY_LEFT_ALT))
	{
		NoraFileHeader fileHeader;
		auto layerInfoVector = NoraFileHandler::readFromDisk("Test.pic", fileHeader);

		layerManager.initWithLayerInfoData(layerInfoVector);


		heightMapTexData.setTextureData(layerInfoVector.at(0).second, fileHeader.width, fileHeader.height, 4);

		heightMapTexData.SetTexId(TextureManager::createTextureFromData(heightMapTexData));
		heightMapTexData.setTextureDirty();
		layerManager.updateLayerTexture(0, heightMapTexData.GetTexId());

		undoRedoSystem.updateAllocation(heightMapTexData.getRes(), heightMapTexData.getComponentCount(), preferencesInfo.maxUndoCount);
		undoRedoSystem.record(heightMapTexData.getTextureData());

	}
}
inline void DisplayBrushSettingsUserInterface(bool &isBlurOn)
{
	ImGui::Text("BRUSH SETTINGS");
	ImGui::Separator();
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 10));
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 5));
	ImGui::Spacing();
	ImGui::PushStyleColor(ImGuiCol_SliderGrab, themeManager->SecondaryColour);
	ImGui::PushStyleColor(ImGuiCol_Button, themeManager->SecondaryColour);

	static std::string currentBrush = "Circle";
	static std::vector<std::string> grungeBrushPaths = fileExplorer->getAllFilesInDirectory(BRUSH_TEXTURES_PATH + "Grunge", false);
	static std::vector<std::string> patternBrushPaths = fileExplorer->getAllFilesInDirectory(BRUSH_TEXTURES_PATH + "Patterns", false);

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
					brushData.textureData.SetTexId(TextureManager::createTextureFromData(brushData.textureData));
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
					brushData.textureData.SetTexId(TextureManager::createTextureFromData(brushData.textureData));
				}
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenu();
	}
	static int item_type;
	ImGui::RadioButton("HEIGHT MODE", &item_type, 0); ImGui::SameLine();
	ImGui::RadioButton("BLUR MODE", &item_type, 1);
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
	ImGui::PushStyleColor(ImGuiCol_Button, themeManager->SecondaryColour);
	if (ImGui::Button("Flip X-Y", ImVec2(ImGui::GetContentRegionAvailWidth(), 40))) { normalViewStateUtility.flipX_Ydir = !normalViewStateUtility.flipX_Ydir; }
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("(CTRL + A)");
	ImGui::PopStyleColor();
	const float width = ImGui::GetContentRegionAvailWidth() / 3.0f - 7;

	if (!normalViewStateUtility.redChannelActive) ImGui::PushStyleColor(ImGuiCol_Button, themeManager->AccentColour1);
	else ImGui::PushStyleColor(ImGuiCol_Button, themeManager->SecondaryColour);
	if (ImGui::Button("R", ImVec2(width, 40))) { normalViewStateUtility.redChannelActive = !normalViewStateUtility.redChannelActive; } ImGui::SameLine();
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("(SHIFT + R)");
	ImGui::PopStyleColor();

	if (!normalViewStateUtility.greenChannelActive) ImGui::PushStyleColor(ImGuiCol_Button, themeManager->AccentColour1);
	else ImGui::PushStyleColor(ImGuiCol_Button, themeManager->SecondaryColour);
	if (ImGui::Button("G", ImVec2(width, 40))) { normalViewStateUtility.greenChannelActive = !normalViewStateUtility.greenChannelActive; } ImGui::SameLine();
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("(SHIFT + G)");
	ImGui::PopStyleColor();

	if (!normalViewStateUtility.blueChannelActive) ImGui::PushStyleColor(ImGuiCol_Button, themeManager->AccentColour1);
	else ImGui::PushStyleColor(ImGuiCol_Button, themeManager->SecondaryColour);
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
	ImGui::PushStyleColor(ImGuiCol_SliderGrab, themeManager->SecondaryColour);
	ImGui::SetNextWindowPos(ImVec2(windowSys.getWindowRes().x - 300, 42), ImGuiSetCond_Always);
	ImGui::SetNextWindowSize(ImVec2(300, windowSys.getWindowRes().y - 77), ImGuiSetCond_Always);
	ImGui::Begin("Preview_Bar", &open, window_flags);

	if (ImGui::ImageButton((ImTextureID)maximizePreviewTexId, ImVec2(80, 40), ImVec2(-0.45f, 1), ImVec2(1.45f, 0), -1, ImVec4(0, 0, 0, 0), themeManager->AccentColour2))
	{
		isPreviewWindowMaximized = true;
		ImGui::OpenPopup("Preview");
	}
	ImGui::SetNextWindowPosCenter();
	ImGui::SetNextWindowSizeConstraints(ImVec2(400, 400), ImVec2(windowSys.getWindowRes().x * 0.95f, windowSys.getWindowRes().y * 0.95f));
	if (ImGui::BeginPopupModal("Preview", &isPreviewWindowMaximized, ImGuiWindowFlags_NoMove))
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
			if (ImGui::IsItemHovered())
				canPerformPreviewWindowMouseOperations = true;
			else
				canPerformPreviewWindowMouseOperations = false;
		}
		else
		{
			ImGui::Dummy(ImVec2(((ImGui::GetContentRegionAvail().x - width) * 0.5f) - 10, (ImGui::GetContentRegionAvail().y - height) * 0.5f));
			ImGui::Image((ImTextureID)previewFbs.getColourTexture(), ImVec2(width, height));
			if (ImGui::IsItemHovered())
				canPerformPreviewWindowMouseOperations = true;
			else
				canPerformPreviewWindowMouseOperations = false;
		}
		ImGui::EndPopup();
	}
	ImGui::SameLine();
	const char* items[] = { "    CUBE", "    CYLINDER", "    SPHERE", "    TORUS","    SUZANNE","    UTAH TEAPOT", "    CUSTOM MODEL" };
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
					modelPreviewObj = modelLoader.createModelFromFile(CUBE_MODEL_PATH);
					break;
				case 1:
					modelPreviewObj = modelLoader.createModelFromFile(CYLINDER_MODEL_PATH);
					break;
				case 2:
					modelPreviewObj = modelLoader.createModelFromFile(SPHERE_MODEL_PATH);
					break;
				case 3:
					modelPreviewObj = modelLoader.createModelFromFile(TORUS_MODEL_PATH);
					break;
				case 4:
					modelPreviewObj = modelLoader.createModelFromFile(COMPLEX_MODELS_PATH + "Suzanne.fbx");
					break;
				case 5:
					modelPreviewObj = modelLoader.createModelFromFile(COMPLEX_MODELS_PATH + "Utah Teapot.fbx");
					break;
				case 6:
					currentLoadingOption = LoadingOption::MODEL;
					fileExplorer->displayDialog(FileType::MODEL, [&](std::string str)
					{
						modelPreviewObj = modelLoader.createModelFromFile(str);
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
	ImGui::Checkbox("Normals", &previewStateUtility.showNormals); ImGui::SameLine();
	float availWidth = ImGui::GetContentRegionAvailWidth() * 0.5f;
	ImGui::PushItemWidth(availWidth);
	if (ImGui::SliderFloat("##Normal Thickness", &previewStateUtility.normDisplayThickness, 1.0f, 10.0f, "Size:%.2f"))
		glLineWidth(previewStateUtility.normDisplayThickness);
	ImGui::SameLine();
	ImGui::SliderFloat("##Normal Length", &previewStateUtility.normDisplayLineLength, 0.1f, 10.0f, "Length:%.2f");
	ImGui::PopItemWidth();

	ImGui::Image((ImTextureID)previewFbs.getColourTexture(), ImVec2(300, 300));
	if (!isPreviewWindowMaximized)
	{
		if (ImGui::IsItemHovered())
			canPerformPreviewWindowMouseOperations = true;
		else
			canPerformPreviewWindowMouseOperations = false;
	}
	ImGui::SliderFloat("##Zoom level", &previewStateUtility.modelPreviewZoomLevel, -1.0f, -100.0f, "Zoom Level:%.2f");
	ImGui::SliderFloat("##Metalness", &previewStateUtility.metalness, 0.01f, 1.0f, "Metalness:%.2f");
	ImGui::SliderFloat("##Roughness", &previewStateUtility.roughness, 0.01f, 10.0f, "Roughness:%.2f");
	ImGui::PopItemWidth();
	ImGui::Spacing();
	ImGui::Text("VIEW MODE");
	ImGui::Separator();
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 10));
	int modeButtonWidth = (int)(ImGui::GetContentRegionAvailWidth() / 3.0f);
	ImGui::Spacing();

	if (previewStateUtility.modelViewMode == 1) ImGui::PushStyleColor(ImGuiCol_Button, themeManager->AccentColour1);
	else ImGui::PushStyleColor(ImGuiCol_Button, themeManager->SecondaryColour);

	if (isUsingLayerOutput)
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
	if (ImGui::Button("Height", ImVec2(modeButtonWidth - 5, 40))) { previewStateUtility.modelViewMode = (isUsingLayerOutput) ? previewStateUtility.modelViewMode : 1; }
	if (isUsingLayerOutput)
		ImGui::PopStyleVar();
	ImGui::PopStyleColor();
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("(Alt + H)");

	ImGui::SameLine(0, 5);
	if (previewStateUtility.modelViewMode == 2) ImGui::PushStyleColor(ImGuiCol_Button, themeManager->AccentColour1);
	else ImGui::PushStyleColor(ImGuiCol_Button, themeManager->SecondaryColour);
	if (ImGui::Button("Normal", ImVec2(modeButtonWidth - 5, 40))) { previewStateUtility.modelViewMode = 2; }
	ImGui::PopStyleColor();
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("(Alt + J)");

	ImGui::SameLine(0, 5);
	if (previewStateUtility.modelViewMode == 3) ImGui::PushStyleColor(ImGuiCol_Button, themeManager->AccentColour1);
	else ImGui::PushStyleColor(ImGuiCol_Button, themeManager->SecondaryColour);
	if (ImGui::Button("Lighting", ImVec2(modeButtonWidth, 40))) { previewStateUtility.modelViewMode = 3; }
	ImGui::PopStyleColor();
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("(Alt + L)");

	ImGui::PopStyleVar();

	if (previewStateUtility.modelViewMode == 2)
	{
		ImGui::Text("MATCAP SETTINGS");
		ImGui::Separator();
		ImGui::Spacing();
		const char* matcapItems[] = { "none", "view space normal", "orange flame", "chrome", "copper", "organic1", "organic3", "organic4",
			"outline1", "outline2", "plastic1", "plastic2", "plastic3" , "platinum", "red metal", "skin1", "skin2" };
		static const char* current_matcap_item = matcapItems[0];
		ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth());
		if (ImGui::BeginCombo("##matcapcombo", current_matcap_item)) // The second parameter is the label previewed before opening the combo.
		{
			for (int n = 0; n < IM_ARRAYSIZE(matcapItems); n++)
			{
				bool is_selected = (current_matcap_item == matcapItems[n]);
				if (ImGui::Selectable(matcapItems[n], is_selected))
				{
					current_matcap_item = matcapItems[n];
					if (current_matcap_item == "none")
						previewStateUtility.useMatcap = false;
					else
					{
						previewStateUtility.useMatcap = true;
						std::string matcapPath = MATCAP_TEXTURES_PATH + current_matcap_item + ".png";
						matcapTexDataForPreview.SetTexId(TextureManager::createTextureFromFile(matcapPath));
					}
				}
				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		ImGui::PopItemWidth();
	}

	if (previewStateUtility.modelViewMode == 3)
	{
		ImGui::Text("LIGHTING SETTINGS");
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth() + 5);
		ImGui::SliderFloat("##Horizontal Position", &previewStateUtility.lightLocation.x, -3.141f, 3.141f, "Horizontal position: %.2f");
		ImGui::SliderFloat("##Vertical position", &previewStateUtility.lightLocation.y, -3.141f, 3.141f, "Vertical position: %.2f");
		ImGui::SliderFloat("##Intensity", &previewStateUtility.lightIntensity, 0.0f, 10.0f, "Intensity: %.2f");

		ImGui::Text("Light Colour"); ImGui::SameLine();
		ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvailWidth() - 40, 10)); ImGui::SameLine();
		ImGui::ColorEdit3("Light Color", &previewStateUtility.lightColour[0], ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoOptions);
		ImGui::Text("Diffuse Colour"); ImGui::SameLine();
		ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvailWidth() - 40, 10)); ImGui::SameLine();
		ImGui::ColorEdit3("Diffuse Color", &previewStateUtility.diffuseColour[0], ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoOptions);
		ImGui::PopItemWidth();

		ImGui::Spacing();
		ImGui::Text("TEXTURE SETTINGS");
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
		ImGui::Text("Albedo");
		ImGui::SameLine(0, 5);
		ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvailWidth() - 80, 10)); ImGui::SameLine();
		if (ImGui::ImageButton((ImTextureID)albedoTexDataForPreview.GetTexId(), ImVec2(40, 40)))
		{
			currentLoadingOption = LoadingOption::TEXTURE;
			fileExplorer->displayDialog(FileType::IMAGE, [&](std::string str)
			{
				if (currentLoadingOption == LoadingOption::TEXTURE)
					albedoTexDataForPreview.SetTexId(TextureManager::createTextureFromFile(str));
			});
		}
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Load albedo map for preview model");
		ImGui::SameLine();
		if (ImGui::Button("X", ImVec2(20, 40))) { albedoTexDataForPreview.SetTexId(defaultWhiteTextureId); }

		ImGui::Text("Metalness");
		ImGui::SameLine(0, 5);
		ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvailWidth() - 80, 10)); ImGui::SameLine();
		if (ImGui::ImageButton((ImTextureID)metalnessTexDataForPreview.GetTexId(), ImVec2(40, 40)))
		{
			currentLoadingOption = LoadingOption::TEXTURE;
			fileExplorer->displayDialog(FileType::IMAGE, [&](std::string str)
			{
				if (currentLoadingOption == LoadingOption::TEXTURE)
					metalnessTexDataForPreview.SetTexId(TextureManager::createTextureFromFile(str));
			});
		}
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Load metalness map for preview model");
		ImGui::SameLine();
		if (ImGui::Button("X##2", ImVec2(20, 40))) { metalnessTexDataForPreview.SetTexId(defaultWhiteTextureId); }

		ImGui::Text("Roughness");
		ImGui::SameLine(0, 5);
		ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvailWidth() - 80, 10)); ImGui::SameLine();
		if (ImGui::ImageButton((ImTextureID)roughnessTexDataForPreview.GetTexId(), ImVec2(40, 40)))
		{
			currentLoadingOption = LoadingOption::TEXTURE;
			fileExplorer->displayDialog(FileType::IMAGE, [&](std::string str)
			{
				if (currentLoadingOption == LoadingOption::TEXTURE)
					roughnessTexDataForPreview.SetTexId(TextureManager::createTextureFromFile(str));
			});
		}
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Load roughness map for preview model");
		ImGui::SameLine();
		if (ImGui::Button("X##3", ImVec2(20, 40))) { roughnessTexDataForPreview.SetTexId(defaultWhiteTextureId); }

		ImGui::PopStyleVar();
	}
	ImGui::End();
	ImGui::PopStyleColor();
	ImGui::PopStyleVar(3);
}
inline void DisplayLayerPanel(const ImGuiWindowFlags &window_flags)
{
	bool open = true;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 1.0f);
	ImGui::PushStyleColor(ImGuiCol_SliderGrab, themeManager->SecondaryColour);
	ImGui::SetNextWindowPos(ImVec2(windowSys.getWindowRes().x - 300, 42), ImGuiSetCond_Always);
	ImGui::SetNextWindowSize(ImVec2(300, windowSys.getWindowRes().y - 77), ImGuiSetCond_Always);
	ImGui::Begin("Layer_Panel", &open, window_flags);

	ImGui::SetNextWindowPosCenter();
	ImGui::SetNextWindowSizeConstraints(ImVec2(400, 400), ImVec2(windowSys.getWindowRes().x * 0.95f, windowSys.getWindowRes().y * 0.95f));

	layerManager.draw();

	ImGui::End();
	ImGui::PopStyleColor();
	ImGui::PopStyleVar(3);
}
inline void DisplayWindowTopBar(unsigned int minimizeTexture, unsigned int restoreTexture, bool &isMaximized, unsigned int closeTexture)
{
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 13));
	if (ImGui::BeginMainMenuBar())
	{
		static bool openPreferences = false;
		ImGui::Indent(10);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(25, 5));
		ImGui::PushFont(menuBarLargerText);
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open Image", "CTRL+O"))
			{
				currentLoadingOption = LoadingOption::TEXTURE;
				fileExplorer->displayDialog(FileType::IMAGE, [&](std::string str)
				{
					if (currentLoadingOption == LoadingOption::TEXTURE)
					{
						TextureManager::getTextureDataFromFile(str, heightMapTexData);
						heightImageLoadLocation = str;
						heightMapTexData.SetTexId(TextureManager::createTextureFromData(heightMapTexData));
						heightMapTexData.setTextureDirty();
						layerManager.updateLayerTexture(0, heightMapTexData.GetTexId());

						undoRedoSystem.updateAllocation(heightMapTexData.getRes(), heightMapTexData.getComponentCount(), preferencesInfo.maxUndoCount);
						undoRedoSystem.record(heightMapTexData.getTextureData());
					}
				});
			}
			if (ImGui::MenuItem("Preferences"))
			{
				openPreferences = true;
			}
			ImGui::EndMenu();
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

		static int item_current = themeManager->getIndexOfTheme(preferencesInfo.defaultTheme);
		static int prev_item = -1;
		ImGui::PushItemWidth(180);
		ImGui::Combo("##combo", &item_current, themeManager->getRawData(), themeManager->getNumberOfThemes());
		ImGui::PopItemWidth();
		ImGui::PopFont();
		if (prev_item != item_current)
		{
			if (item_current >= 3)
				themeManager->setThemeFromFile(THEMES_PATH + (std::string(themeManager->getRawData()[item_current]) + ".nort"));
			else
			{
				if (item_current == 0)
					themeManager->enableInBuiltTheme(ThemeManager::Theme::DEFAULT);
				else if (item_current == 1)
					themeManager->enableInBuiltTheme(ThemeManager::Theme::DARK);
				else if (item_current == 2)
					themeManager->enableInBuiltTheme(ThemeManager::Theme::LIGHT);
			}
		}
		prev_item = item_current;
		ImGui::PopStyleVar();

		ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvailWidth() - 300, 0));
		if (ImGui::Button("Preview", ImVec2(142, 40)))
		{
			isPreviewPanelActive = true;
		}
		if (ImGui::Button("Layers", ImVec2(142, 40)))
		{
			isPreviewPanelActive = false;
		}

		ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowWidth() * 0.5f - 150, ImGui::GetWindowHeight() * 0.5f + 300));
		if (openPreferences)
			ImGui::OpenPopup("Preferences");
		static PreferenceInfo info = preferencesInfo;
		static int res[2] = { info.maxWidthRes, info.maxHeightRes };
		static int stepNum = info.maxUndoCount;
		static char defaultPath[500] = { '\0' };
		if (defaultPath[0] == '\0')
			std::memcpy(defaultPath, &info.defaultExportPath[0], info.defaultExportPath.size());
		if (ImGui::BeginPopupModal("Preferences", &openPreferences, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize))
		{
			ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth());
			ImGui::Text("Max image resolution :");

			ImGui::InputInt2("##Dimensions", &res[0]);
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Nora needs to allocate memory at start of application and needs to know the max size of an image that will be opened");
			ImGui::Text("Max undo Steps :");
			ImGui::InputInt("##Undo step count", &stepNum, 1, 1);
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("RAM is allocated for Undo/Redo operations, To minimize RAM usage set max step count to low value");
			stepNum = glm::max(stepNum, 10);
			ImGui::Text("Set default export path :");
			ImGui::InputText("##Path", defaultPath, 500);
			ImGui::Text("Set default theme :");
			static int preference_item_current = item_current;
			ImGui::Combo("##combo", &preference_item_current, themeManager->getRawData(), themeManager->getNumberOfThemes());
			ImGui::Text("These changes take effect on next application start up");
			if (ImGui::Button("Save Perferences", ImVec2(ImGui::GetContentRegionAvailWidth()*0.5f, 40)))
			{
				PreferencesHandler::savePreferences(res[0], res[1], stepNum, defaultPath, std::string(themeManager->getRawData()[preference_item_current]));
			}
			ImGui::SameLine();
			if (ImGui::Button("Reset to defaults", ImVec2(ImGui::GetContentRegionAvailWidth(), 40)))
			{
				res[0] = 4096;
				res[1] = 4096;
				stepNum = 20;
				preference_item_current = 0;
			}
			ImGui::PopItemWidth();
			ImGui::EndPopup();
		}
	}
	ImGui::EndMainMenuBar();
	ImGui::PopStyleVar();
}
void SaveNormalMapToFile(const std::string &locationStr, ImageFormat imageFormat)
{
	if (locationStr.length() > 4)
	{
		fbs.bindColourTexture();
		glPixelStorei(GL_PACK_ALIGNMENT, 1);

		const int nSize = static_cast<int>(heightMapTexData.getRes().x * heightMapTexData.getRes().y * 3);
		char* dataBuffer = new char[nSize];
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, dataBuffer);

		glBindTexture(GL_TEXTURE_2D, 0);
		fbs.updateTextureDimensions(windowSys.getWindowRes().x, windowSys.getWindowRes().y);
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
	const glm::vec2 currentMouseCoord = windowSys.getCursorPos();

	if (state == GLFW_PRESS && !fileExplorer->shouldDisplay)
	{
		const glm::vec2 wnCurMouse = glm::vec2(currentMouseCoord.x / windowSys.getWindowRes().x, 1.0f - currentMouseCoord.y / windowSys.getWindowRes().y);
		const glm::vec2 wnPrevMouse = glm::vec2(prevMouseCoord.x / windowSys.getWindowRes().x, 1.0f - prevMouseCoord.y / windowSys.getWindowRes().y);
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
						int numberOfPoints = static_cast<int>(glm::floor(glm::clamp(glm::length(diff) / density, 1.0f, 300.0f)));

						for (int i = 0; i < numberOfPoints; i++)
						{
							const float left = (iterCurPoint.x - convertedBrushScale.x) * maxWidth;
							const float right = (iterCurPoint.x + convertedBrushScale.x) * maxWidth;
							const float bottom = (iterCurPoint.y - convertedBrushScale.y) * maxHeight;
							const float top = (iterCurPoint.y + convertedBrushScale.y) * maxHeight;
							iterCurPoint += incValue;
							if (brushData.hasBrushTexture())
								SetPixelValuesWithBrushTexture(heightMapTexData, brushData.textureData, left, right, bottom, top, iterCurPoint.x, iterCurPoint.y);
							else
								SetPixelValues(heightMapTexData, left, right, bottom, top, iterCurPoint.x, iterCurPoint.y);
						}
					}
					else
					{
						const float left = (curX - convertedBrushScale.x) * maxWidth;
						const float right = (curX + convertedBrushScale.x) * maxWidth;
						const float bottom = (curY - convertedBrushScale.y) * maxHeight;
						const float top = (curY + convertedBrushScale.y) * maxHeight;

						if (brushData.hasBrushTexture())
							SetPixelValuesWithBrushTexture(heightMapTexData, brushData.textureData, left, right, bottom, top, curX, curY);
						else
							SetPixelValues(heightMapTexData, left, right, bottom, top, curX, curY);
					}
				}
				else if (isBlurOn)
				{
					const float left = (curX - convertedBrushScale.x) * maxWidth;
					const float right = (curX + convertedBrushScale.x) * maxWidth;
					const float bottom = (curY - convertedBrushScale.y) * maxHeight;
					const float top = (curY + convertedBrushScale.y) * maxHeight;
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
			prevMouseCoord = INVALID;
			undoRedoSystem.record(heightMapTexData.getTextureData());
			didActuallyDraw = false;
		}
	}
	prevState = state;
}
inline void HandleMiddleMouseButtonInput(int state, glm::vec2 &prevMiddleMouseButtonCoord, double deltaTime, DrawingPanel &frameBufferPanel)
{
	if (state == GLFW_PRESS)
	{
		glm::vec2 currentPos = windowSys.getCursorPos();
		glm::vec2 diff = (currentPos - prevMiddleMouseButtonCoord) * glm::vec2(1.0f / windowSys.getWindowRes().x, 1.0f / windowSys.getWindowRes().y) * 2.0f;
		frameBufferPanel.getTransform()->translate(diff.x, -diff.y);
		prevMiddleMouseButtonCoord = currentPos;
	}
	else
	{
		prevMiddleMouseButtonCoord = windowSys.getCursorPos();
	}
}
inline void SetPixelValues(TextureData& inputTexData, int startX, int endX, int startY, int endY, double xpos, double ypos)
{
	const glm::vec2 pixelPos(xpos, ypos);
	const float px_width = inputTexData.getRes().x;
	const float px_height = inputTexData.getRes().y;
	const float distanceRemap = 1.0f / brushData.brushScale;
	const float offsetRemap = glm::pow(brushData.brushOffset, 2) * 10.0f;

	const float xMag = static_cast<float>(endX - startX);
	const float yMag = static_cast<float>(endY - startY);

	const int clampedStartX = glm::max(startX, 0);
	const int clampedEndX = glm::min(endX, (int)inputTexData.getRes().x);
	const int clampedStartY = glm::clamp(startY, 0, (int)inputTexData.getRes().y);
	const int clampedEndY = glm::clamp(endY, 0, (int)inputTexData.getRes().y);

	for (int i = clampedStartX; i < clampedEndX; i++)
	{
		for (int j = clampedStartY; j < clampedEndY; j++)
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
				rVal = rVal + distance * ((brushData.heightMapPositiveDir ? brushData.brushMaxHeight : brushData.brushMinHeight) - rVal);
				ColourData col(rVal, rVal, rVal, 1.0f);
				inputTexData.setTexelColor(col, i, j);
			}
		}
	}
}
inline void SetPixelValuesWithBrushTexture(TextureData& inputTexData, TextureData& brushTexture, int startX, int endX, int startY, int endY, double xpos, double ypos)
{
	const float xMag = static_cast<float>(endX - startX);
	const float yMag = static_cast<float>(endY - startY);

	const int clampedStartX = glm::max(startX, 0);
	const int clampedEndX = glm::min(endX, (int)inputTexData.getRes().x);
	const int clampedStartY = glm::clamp(startY, 0, (int)inputTexData.getRes().y);
	const int clampedEndY = glm::clamp(endY, 0, (int)inputTexData.getRes().y);

	for (int i = clampedStartX; i < clampedEndX; i++)
	{
		for (int j = clampedStartY; j < clampedEndY; j++)
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
	//Crashes when drawing with blur at bottom of panel

	const int imageWidth = static_cast<int>(inputTexData.getRes().x);
	const int imageHeight = static_cast<int>(inputTexData.getRes().y);
	//Temp allocation of image section

	const int clampedStartX = glm::max(startX, 0);
	const int clampedEndX = glm::min(endX, (int)inputTexData.getRes().x);
	const int clampedStartY = glm::clamp(startY, 0, (int)inputTexData.getRes().y);
	const int clampedEndY = glm::clamp(endY, 0, (int)inputTexData.getRes().y);

	const int _width = endX - startX;
	const int _height = endY - startY;
	const int totalPixelCount = _width * _height;
	ColourData **tempPixelData = new ColourData*[_width];
	//std::memset(tempPixelData, 1.0f, sizeof(float)*totalPixelCount * 4);

	for (int i = startX; i < endX; i++)
	{
		tempPixelData[i - startX] = new ColourData[_height];
		for (int j = startY; j < endY; j++)
		{
			if (i >= clampedStartX && i < clampedEndX && j >= clampedStartY && j < clampedEndY)
				tempPixelData[i - startX][j - startY] = inputTexData.getTexelColor(i, j);
		}
	}

	const float xMag = static_cast<float>(endX - startX);
	const float yMag = static_cast<float>(endY - startY);

	for (int i = clampedStartX; i < clampedEndX; i++)
	{
		for (int j = clampedStartY; j < clampedEndY; j++)
		{
			const float x = (i - startX) / xMag;
			const float y = (j - startY) / yMag;
			float distance = glm::distance(glm::vec2(0), glm::vec2(x * 2.0f - 1.0f, y * 2.0f - 1.0f));
			distance = glm::clamp(distance, 0.0f, 1.0f);
			if (distance < 1.0f)
			{
				if (i - 1 < startX || i + 1 > endX || j - 1 < startY || j + 1 > endY)
					continue;

				float pixelCol = tempPixelData[i - startX][j - startY].getColour_32_Bit().r;
				float validEntries = 0;
				float neighbourAvg = 0;

				for (int xoffset = -1; xoffset <= 1; xoffset++)
				{
					for (int yoffset = -1; yoffset <= 1; yoffset++)
					{
						int xIndex = (i + xoffset) - startX;
						int yIndex = (j + yoffset) - startY;
						if (xIndex >= 0 && xIndex < _width && yIndex >= 0 && yIndex < _height)
						{
							validEntries++;
							neighbourAvg += tempPixelData[xIndex][yIndex].getColour_32_Bit().r;
						}
					}
				}
				float avg = (neighbourAvg / validEntries);
				float finalColor = avg;
				finalColor = glm::mix(pixelCol, finalColor, brushData.brushStrength);
				finalColor = glm::clamp(finalColor, 0.0f, 1.0f);
				ColourData colData;
				colData.setColour_32_bit(glm::vec4(finalColor, finalColor, finalColor, 1.0f));
				inputTexData.setTexelColor(colData, i, j);
			}
		}
	}
	for (int i = 0; i < _width; i++)
		delete[] tempPixelData[i];
	delete[] tempPixelData;
	tempPixelData = nullptr;
}
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	width = glm::clamp(width, windowSys.getMinWindowSize(), (int)windowSys.getMaxWindowRes().x);
	height = glm::clamp(height, windowSys.getMinWindowSize(), (int)windowSys.getMaxWindowRes().y);

	windowSys.setWindowRes(width, height);
	fbs.updateTextureDimensions(windowSys.getWindowRes());
	layerManager.updateFramebufferTextureDimensions(windowSys.getWindowRes());
	previewFbs.updateTextureDimensions(windowSys.getWindowRes());
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) noexcept
{
	if (!canPerformPreviewWindowMouseOperations)
		normalViewStateUtility.zoomLevel += normalViewStateUtility.zoomLevel * 0.1f * yoffset;
	else if (canPerformPreviewWindowMouseOperations)
		previewStateUtility.modelPreviewZoomLevel += 0.5f * yoffset;
}
//struct BoundsAndPos
//{
//public:
//	glm::vec2 mouseCoord;
//	float left, right, bottom, up;
//};
//
//std::queue<BoundsAndPos> mouseCoordQueue;
//
//
//void ApplyChangesToPanel()
//{
//	const float maxWidth = heightMapTexData.getRes().x;
//	const float convertedBrushScale = (brushData.brushScale / heightMapTexData.getRes().y) * maxWidth * 3.5f;
//
//	while (!windowSys.IsWindowClosing())
//	{
//		while (mouseCoordQueue.size() > 0)
//		{
//			BoundsAndPos boundAndPos = mouseCoordQueue.front();
//			mouseCoordQueue.pop();
//			glm::vec2 mousePos = boundAndPos.mouseCoord;
//			glm::vec2 prevMousePos = mouseCoordQueue.front().mouseCoord;
//
//			glm::vec2 curPoint = prevMousePos;
//			glm::vec2 incValue = (prevMousePos - mousePos) * 0.333f;
//			curPoint += incValue;
//
//			for (int i = 0; i < 3; i++)
//			{
//				float left = boundAndPos.left;
//				float right = boundAndPos.right;
//				float bottom = boundAndPos.bottom;
//				float top = boundAndPos.up;
//
//				left = glm::clamp(left, 0.0f, maxWidth);
//				right = glm::clamp(right, 0.0f, maxWidth);
//				bottom = glm::clamp(bottom, 0.0f, maxWidth);
//				top = glm::clamp(top, 0.0f, maxWidth);
//
//				curPoint += incValue;
//				SetPixelValues(heightMapTexData, left, right, bottom, top, curPoint.x, curPoint.y);
//			}
//			//SetPixelValues(heightMapTexData, boundAndPos.left, boundAndPos.right, boundAndPos.bottom, boundAndPos.up, mousePos.x, mousePos.y, brushData);
//		}
//		std::this_thread::sleep_for(std::chrono::milliseconds(1));
//	}
//}