#include "LayerManager.h"

LayerManager::LayerManager()
{
}

LayerManager::~LayerManager()
{
}

void LayerManager::addLayer(int texId, LayerType layerType, const std::string& layerName)
{
	LayerInfo layerInfo;
	layerInfo.textureId = texId;
	if (layerName == "")
		layerInfo.layerName = "Layer" + std::to_string(layers.size());
	else
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
		char* buffer = &layers.at(i).layerName[0];
		ImGui::InputText("##Layer Name", buffer, layers.at(i).layerName.size());
		const char* items[] = { "Height Map", "Normal Map" };
		int item_current = (int)layers.at(i).layerType;
		if (ImGui::Combo("##combo" + i, &item_current, items, IM_ARRAYSIZE(items)))
		{
			switch (item_current)
			{
			case 0:
				layers.at(i).layerType = LayerType::HEIGHT_MAP;
				break;
			case 1:
				layers.at(i).layerType = LayerType::NORMAL_MAP;
				break;
			default:
				break;
			}
		}
	}
}
