#pragma once
#include <string>
#include <vector>
#include <GLM\glm.hpp>
#include "FrameBufferSystem.h"
#include "ImGui\imgui.h"

enum class LayerType { HEIGHT_MAP=0, NORMAL_MAP };

struct LayerInfo
{
	int inputTextureId = 0;
	unsigned int outputTextureId = 0;
	char* layerName;
	LayerType layerType;
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
	void addLayer(int texId, LayerType layerType = LayerType::HEIGHT_MAP, const std::string& layerName = "");
	void updateFramebufferTextureDimensions(const glm::vec2 resolution);
	LayerType getLayerType(int index);
	void bindFrameBuffer(int index);
	unsigned int getColourTexture(int index);
	int getInputTexId(int index);
	void draw();
};

