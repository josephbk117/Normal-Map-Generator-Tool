#pragma once
#include <string>
#include <vector>
#include "ImGui\imgui.h"

enum class LayerType { HEIGHT_MAP, NORMAL_MAP };

struct LayerInfo
{
	int inputTextureId = 0;
	unsigned int outputTextureId = 0;
	std::string layerName = "Layer 1";
	LayerType layerType;
};

class LayerManager
{
private:
	std::vector<LayerInfo> layers;
public:
	LayerManager();
	~LayerManager();
	void addLayer(int texId, LayerType layerType = LayerType::HEIGHT_MAP, const std::string& layerName = "");
	void setOutputTexture(int index,unsigned int texId);
	int getInputTexId(int index);
	std::string* getLayerNameAddress(int index);
	void draw();
};

