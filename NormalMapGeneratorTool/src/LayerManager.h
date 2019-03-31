#pragma once
#include <string>
#include <vector>
#include <GLM\glm.hpp>
#include "FrameBufferSystem.h"
#include "ImGui\imgui.h"

enum class LayerType { HEIGHT_MAP=0, NORMAL_MAP };

struct LayerInfo
{
	unsigned int inputTextureId = 0;
	unsigned int outputTextureId = 0;
	char* layerName;
	bool isActive;
	float strength;
	LayerType layerType = LayerType::HEIGHT_MAP;
	FrameBufferSystem fbs;
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
	int getLayerCount();
	void init(const glm::vec2 & windowRes, const glm::vec2& maxBufferResolution);
	void updateLayerTexture(int index, unsigned int textureId);
	void addLayer(int texId, LayerType layerType = LayerType::HEIGHT_MAP, const std::string& layerName = "");
	void setLayerActiveState(int index, bool isActive);
	bool isLayerActive(int index);
	void updateFramebufferTextureDimensions(const glm::vec2 resolution);
	float getLayerStrength(int index);
	LayerType getLayerType(int index);
	void bindFrameBuffer(int index);
	unsigned int getColourTexture(int index);
	int getInputTexId(int index);
	void draw();
};

