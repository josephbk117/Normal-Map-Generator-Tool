#pragma once
#include <string>
#include <vector>
#include <GLM\glm.hpp>
#include "FrameBufferSystem.h"
#include "ImGui\imgui.h"

enum class LayerType { HEIGHT_MAP, NORMAL_MAP };

struct LayerInfo
{
	int inputTextureId = 0;
	unsigned int outputTextureId = 0;
	std::string layerName = "Layer 1";
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
	~LayerManager() {}
	int getLayerCount();
	void init(const glm::vec2 & windowRes, const glm::vec2& maxBufferResolution);
	void addLayer(int texId, LayerType layerType = LayerType::HEIGHT_MAP, const std::string& layerName = "");
	void updateFramebufferTextureDimensions(const glm::vec2 resolution);
	void bindFrameBuffer(int index);
	unsigned int getColourTexture(int index);
	int getInputTexId(int index);
	std::string* getLayerNameAddress(int index);
	void draw();
};

