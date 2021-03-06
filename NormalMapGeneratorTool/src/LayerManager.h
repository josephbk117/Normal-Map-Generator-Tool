#pragma once
#include <string>
#include <vector>
#include <GLM\glm.hpp>
#include "FrameBufferSystem.h"
#include "TextureLoader.h"
#include "ImGui\imgui.h"

struct LayerInfoData;
enum class LayerType { HEIGHT_MAP = 0, NORMAL_MAP };
enum class NormalBlendMethod { REORIENTED_NORMAL_BLENDING = 0, UNREAL_NORMAL_BLENDING, PARTIAL_DERIVATIVE_NORMAL_BLENDING };

struct LayerInfo
{
	unsigned int inputTextureId = 0;
	unsigned int outputTextureId = 0;
	char* layerName = nullptr;
	bool isActive;
	float strength;
	LayerType layerType = LayerType::HEIGHT_MAP;
	NormalBlendMethod normalBlendMethod = NormalBlendMethod::REORIENTED_NORMAL_BLENDING;
	FrameBufferSystem fbs;
	glm::vec2 resolution;
	ImageFormat imageFormat = ImageFormat::UNCOMPRESSED_RAW;
	std::string imagePath;
};

class LayerManager
{
private:
	std::vector<LayerInfo> layers;
	glm::vec2 windowRes;
	glm::vec2 maxBufferResolution;
public:
	LayerManager() {}
	~LayerManager();
	int getLayerCount()const;
	void init(const glm::vec2 & windowRes, const glm::vec2& maxBufferResolution);
	void initWithLayerInfoData(const std::vector<std::pair<LayerInfoData, unsigned char*>>& layerInfoData);
	void updateLayerTexture(int index, unsigned int textureId);
	void addLayer(int texId, LayerType layerType = LayerType::HEIGHT_MAP, const std::string& layerName = "", const std::string& imagePath = "");
	void setLayerActiveState(int index, bool isActive);
	NormalBlendMethod getNormalBlendMethod(int index)const;
	bool isLayerActive(int index)const;
	void updateFramebufferTextureDimensions(const glm::vec2 resolution);
	float getLayerStrength(int index)const;
	LayerType getLayerType(int index)const;
	void bindFrameBuffer(int index);
	std::string getImagePath(int index)const;
	unsigned int getColourTexture(int index)const;
	unsigned int getInputTexId(int index)const;
	char* getLayerName(int index)const;
	void draw();
};

