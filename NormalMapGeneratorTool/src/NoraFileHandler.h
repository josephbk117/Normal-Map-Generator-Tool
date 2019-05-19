#pragma once
#include <iostream>
#include <fstream>
#include "LayerManager.h"
#include "TextureLoader.h"
#include "TextureData.h"
#include "FileExplorer.h"
#include "Stb\stb_image.h"
/*
Nora save file : .nora
Header Defenition:
nora (text)
[MAJOR VERSION] (unsigned short)
[MINOR VERSION] (unsigned short)
[NUMBER OF LAYERS] (unsigned int)
------------------
Layer info:
[LAYER NAME SIZE] (unsigned int)
[LAYER NAME] (char...)
[LAYER TYPE] (LayerType)
[BLEND MODE] (BlendMode)
[LAYER STRENGTH] (float)
[IMAGE FORMAT] (ImageFormat)
[DATA SIZE (in bytes)] (unsigned long)
////[DATA] (char...)
*/

struct LayerInfoData
{
	char layerName[100];
	LayerType layerType;
	NormalBlendMethod blendMode;
	float layerStrength;
	ImageFormat format;
	unsigned long dataSize;
};

struct NoraFileHeader
{
	char nora[5];
	unsigned short majorVersion;
	unsigned short minorVersion;
	unsigned int numberOfLayers;
	unsigned int width;
	unsigned int height;
};

class NoraFileHandler
{
public:
	static void writeToDisk(const std::string& path, const TextureData& texData, const LayerManager& layerManager)
	{
		NoraFileHeader noraFileHeader;
		noraFileHeader.nora[0] = 'n';
		noraFileHeader.nora[1] = 'o';
		noraFileHeader.nora[2] = 'r';
		noraFileHeader.nora[3] = 'a';
		noraFileHeader.nora[4] = '\0';

		noraFileHeader.majorVersion = 1;
		noraFileHeader.minorVersion = 4;
		noraFileHeader.numberOfLayers = layerManager.getLayerCount();
		noraFileHeader.width = static_cast<unsigned int>(texData.getRes().x);
		noraFileHeader.height = static_cast<unsigned int>(texData.getRes().y);

		std::vector<std::pair<LayerInfoData, unsigned char*>> layerInfos;

		for (unsigned int i = 0; i < noraFileHeader.numberOfLayers; i++)
		{
			std::pair<LayerInfoData, unsigned char*> layerInfoPair;
			std::memcpy(layerInfoPair.first.layerName, layerManager.getLayerName(i), 100);
			layerInfoPair.first.layerType = layerManager.getLayerType(i);
			layerInfoPair.first.blendMode = layerManager.getNormalBlendMethod(i);
			layerInfoPair.first.layerStrength = layerManager.getLayerStrength(i);
			layerInfoPair.first.format = ImageFormat::PNG;

			if (i > 0)
			{
				std::ifstream ifs(layerManager.getImagePath(i), std::ios::binary | std::ios::ate);
				std::ifstream::pos_type pos = ifs.tellg();
				layerInfoPair.first.dataSize = pos;
				layerInfoPair.second = new unsigned char[pos];
				ifs.seekg(0, std::ios::beg);
				ifs.read((char*)(&layerInfoPair.second[0]), pos);
				ifs.close();
			}
			else
			{
				layerInfoPair.first.dataSize = static_cast<unsigned long>(texData.getRes().x * texData.getRes().y * texData.getComponentCount());
				layerInfoPair.second = new unsigned char[layerInfoPair.first.dataSize];
				std::memset(layerInfoPair.second, '\0', layerInfoPair.first.dataSize);
				std::memcpy(layerInfoPair.second, texData.getTextureData(), layerInfoPair.first.dataSize);
			}
			layerInfos.push_back(layerInfoPair);
		}
		std::ofstream myfile(path.c_str(), std::ios::binary);
		myfile.write((char*)&noraFileHeader, sizeof(NoraFileHeader));

		for (unsigned int i = 0; i < noraFileHeader.numberOfLayers; i++)
		{
			myfile.write((char*)&layerInfos[i].first, sizeof(LayerInfoData));
			myfile.write((char*)(&layerInfos[i].second[0]), layerInfos[i].first.dataSize);
		}
		myfile.close();
		for (unsigned int i = 0; i < noraFileHeader.numberOfLayers; i++)
			delete[] layerInfos[i].second;
	}

	static std::vector<std::pair<LayerInfoData, unsigned char*>> readFromDisk(const std::string& path, NoraFileHeader& noraFile)
	{
		LayerInfoData info;
		std::vector<std::pair<LayerInfoData, unsigned char*>> layerInfos;
		std::ifstream myfile(path.c_str(), std::ios::binary);
		myfile.read((char*)&noraFile, sizeof(NoraFileHeader));
		for (unsigned int i = 0; i < noraFile.numberOfLayers; i++)
		{
			myfile.read((char*)&info, sizeof(LayerInfoData));
			unsigned char* data = new unsigned char[info.dataSize];
			myfile.read((char*)(&data[0]), info.dataSize);

			int x, y, n;
			stbi_info_from_memory(data, info.dataSize, &x, &y, &n);
			layerInfos.push_back(std::pair<LayerInfoData, unsigned char*>(info, data));
		}
		myfile.close();
		return layerInfos;
	}
};