#include "LayerManager.h"
#include "GL\glew.h"
LayerManager::LayerManager()
{
}

LayerManager::~LayerManager()
{
}

void LayerManager::addLayer(int texId, LayerType layerType, const std::string& layerName)
{
	LayerInfo layerInfo;
	layerInfo.inputTextureId = texId;
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
		ImGui::Image((ImTextureID)layers.at(i).outputTextureId, ImVec2(50, 50)); ImGui::SameLine();
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
void LayerManager::setOutputTexture(int index, unsigned int texId)
{
	/*if (layers.at(index).outputTextureId == 0)
		glGenTextures(1, &layers.at(index).outputTextureId);

	glReadBuffer(GL_COLOR_ATTACHMENT0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, layers.at(index).outputTextureId);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 512, 512, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, 512, 512);
	glBindTexture(GL_TEXTURE_2D, 0);*/
	layers.at(index).outputTextureId = texId;
}
int LayerManager::getInputTexId(int index)
{
	return layers.at(index).inputTextureId;
}
