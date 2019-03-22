#pragma once
#include <string>
#include <vector>
#include "ImGui\imgui.h"
struct LayerInfo
{
	int textureId;
	std::string layerName = "Layer 1";
};

class LayerManager
{
private:
	std::vector<LayerInfo> layers;
public:
	LayerManager();
	~LayerManager();
	void addLayer(int texId, std::string layerName);
	std::string* getLayerNameAddress(int index);
	void draw();
};

