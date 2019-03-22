#include "LayerManager.h"

LayerManager::LayerManager()
{
}

LayerManager::~LayerManager()
{
}

void LayerManager::addLayer(int texId, std::string layerName)
{
	LayerInfo layerInfo;
	layerInfo.textureId = texId;
	layerInfo.layerName = layerName;
	layers.push_back(layerInfo);
}
std::string* LayerManager::getLayerNameAddress(int index)
{
	return &layers.at(index).layerName;
}
void LayerManager::draw()
{
	for (unsigned int i = 0; i < layers.size(); i++)
	{
		ImGui::Image((ImTextureID)layers.at(i).textureId, ImVec2(50, 50)); ImGui::SameLine();
		static char buffer[200] = "Layer 1";
		ImGui::InputText("##Layer Name", buffer, 200);
	}
}
