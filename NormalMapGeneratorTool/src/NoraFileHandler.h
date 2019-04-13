#pragma once
#include <iostream>
#include <fstream>
#include "LayerManager.h"
#include "TextureLoader.h"
#include "TextureData.h"

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
				layerInfoPair.first.dataSize = 3;
				layerInfoPair.second = new unsigned char[layerInfoPair.first.dataSize];
				std::memset(layerInfoPair.second, '\0', layerInfoPair.first.dataSize);
				layerInfoPair.second[0] = 'g';
				layerInfoPair.second[1] = 'k';
				layerInfoPair.second[2] = '\0';
			}
			else
			{
				layerInfoPair.first.dataSize = texData.getRes().x * texData.getRes().y * texData.getComponentCount();
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
			myfile.write((char*)layerInfos[i].second, layerInfos[i].first.dataSize);
		}

		myfile.close();

		for (unsigned int i = 0; i < noraFileHeader.numberOfLayers; i++)
			delete[] layerInfos[i].second;
	}

	static void readFromDisk(const std::string& path)
	{
		NoraFileHeader noraFile;
		LayerInfoData info;

		std::vector<std::pair<LayerInfoData, unsigned char*>> layerInfos;

		std::ifstream myfile(path.c_str(), std::ios::binary);
		myfile.read((char*)&noraFile, sizeof(NoraFileHeader));

		std::cout << "\n Nora header" << noraFile.nora;
		std::cout << "\n Major version " << noraFile.majorVersion;
		std::cout << "\n Minor version " << noraFile.minorVersion;
		std::cout << "\n Number of layers " << noraFile.numberOfLayers;

		for (unsigned int i = 0; i < noraFile.numberOfLayers; i++)
		{
			myfile.read((char*)&info, sizeof(LayerInfoData));

			std::cout << "\n-Layer " << i << "name " << info.layerName;
			std::cout << "\n-Layer " << i << "layer type " << (int)info.layerType;
			std::cout << "\n-Layer " << i << "blend mode " << (int)info.blendMode;
			std::cout << "\n-Layer " << i << "strength " << info.layerStrength;
			std::cout << "\n-Layer " << i << "format " << (int)info.format;
			std::cout << "\n-Layer " << i << "data size " << info.dataSize;

			unsigned char* data = new unsigned char[info.dataSize];
			std::memset(data, '\0', info.dataSize);
			myfile.read((char*)data, info.dataSize);
			layerInfos.push_back(std::pair<LayerInfoData, unsigned char*>(info, data));
			std::cout << "\nLayer" << i << " data " << data;
		}

		/*	myfile.read((char*)&info, sizeof(LayerInfoData));

			std::cout << "\n-Layer 2 name " << info.layerName;
			std::cout << "\n-Layer 2 layer type " << (int)info.layerType;
			std::cout << "\n-Layer 2 blend mode " << (int)info.blendMode;
			std::cout << "\n-Layer 2 strength " << info.layerStrength;
			std::cout << "\n-Layer 2 format " << (int)info.format;
			std::cout << "\n-Layer 2 data size " << info.dataSize;

			data = new unsigned char[info.dataSize];
			std::memset(data, '\0', info.dataSize);

			myfile.read((char*)data, info.dataSize);
			std::cout << "\nLayer 2 data " << data;

			delete[] data;*/
		myfile.close();

	}
};