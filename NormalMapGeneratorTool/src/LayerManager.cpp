#include <set>
#include <iostream>
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
	std::set<unsigned int> markedForDeletionLayerIndices;
	for (unsigned int i = 0; i < layers.size(); i++)
	{
		ImGui::Image((ImTextureID)layers.at(i).fbs.getColourTexture(), ImVec2(50, 50)); ImGui::SameLine();
		char* buffer = &layers.at(i).layerName[0];
		std::string inputText = "##Input text";
		inputText += std::to_string(i);
		ImGui::InputText(inputText.c_str(), buffer, layers.at(i).layerName.size());
		const char* items[] = { "Height Map", "Normal Map" };
		int item_current = (int)layers.at(i).layerType;
		std::string comboBoxName = "##combo";
		comboBoxName += std::to_string(i);
		if (ImGui::Combo(comboBoxName.c_str(), &item_current, items, IM_ARRAYSIZE(items)))
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
		std::string removeLayerButtonName = "Remove Layer ";
		removeLayerButtonName += std::to_string(i);
		if (ImGui::Button(removeLayerButtonName.c_str(), ImVec2(ImGui::GetContentRegionAvailWidth(), 30)))
		{
			markedForDeletionLayerIndices.insert(i);
		}
	}
	if (ImGui::Button("Add Layer", ImVec2(ImGui::GetContentRegionAvailWidth(), 40)))
	{
		std::cout << "\nAdd layer called";
	}
	std::set<unsigned int>::iterator it;
	for (it = markedForDeletionLayerIndices.begin(); it != markedForDeletionLayerIndices.end(); it++)
	{
		std::cout << "\n Remove at " << *it;
		layers.erase(layers.begin() + *it);
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