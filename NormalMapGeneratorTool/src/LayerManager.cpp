#include "LayerManager.h"
#include "GL\glew.h"

int LayerManager::getLayerCount()
{
	return layers.size();
}

void LayerManager::init(const glm::vec2 & windowRes, const glm::vec2& maxBufferResolution)
{
	this->windowRes = windowRes;
	this->maxBufferResolution = maxBufferResolution;
}

void LayerManager::addLayer(int texId, LayerType layerType, const std::string& layerName)
{
	LayerInfo layerInfo;
	layerInfo.inputTextureId = texId;
	if (layerName == "")
		layerInfo.layerName = "Layer" + std::to_string(layers.size());
	else
		layerInfo.layerName = layerName;
	layerInfo.fbs.init(windowRes, maxBufferResolution);
	layers.push_back(layerInfo);
}
void LayerManager::updateFramebufferTextureDimensions(const glm::vec2 resolution)
{
	for (int layerIndex = 0; layerIndex < layers.size(); layerIndex++)
		layers.at(layerIndex).fbs.updateTextureDimensions(resolution);
}
LayerType LayerManager::getLayerType(int index)
{
	return layers.at(index).layerType;
}
std::string* LayerManager::getLayerNameAddress(int index)
{
	return &layers.at(index).layerName;
}
void LayerManager::draw()
{
	for (unsigned int i = 0; i < layers.size(); i++)
	{
		ImGui::Image((ImTextureID)layers.at(i).fbs.getColourTexture(), ImVec2(50, 50)); ImGui::SameLine();
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

void LayerManager::bindFrameBuffer(int index)
{
	layers.at(index).fbs.bindFrameBuffer();
}

unsigned int LayerManager::getColourTexture(int index)
{
	return layers.at(index).fbs.getColourTexture();
}

int LayerManager::getInputTexId(int index)
{
	return layers.at(index).inputTextureId;
}
