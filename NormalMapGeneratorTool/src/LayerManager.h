#pragma once
#include <string>
#include <vector>
#include "ImGui\imgui.h"

enum class LayerType { HEIGHT_MAP, NORMAL_MAP };

struct LayerInfo
{
	int textureId;
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
	void addLayer(int texId, LayerType layerType = LayerType::HEIGHT_MAP, const std::string& layerName="");
	std::string* getLayerNameAddress(int index);
	void draw();
};

