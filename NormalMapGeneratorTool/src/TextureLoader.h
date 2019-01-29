#pragma once
#include "TextureData.h"
#include <GL\glew.h>
#include <unordered_map>
#include <vector>
#include <GLM\common.hpp>
enum class ImageFormat { PNG = 100, JPEG, BMP, TGA };

class TextureManager
{
public:
	TextureManager() = delete;
	TextureManager(const TextureManager&) = delete;
	static void getRawImageDataFromFile(const std::string& path, std::vector<unsigned char>& data, int &width, int &height, bool flipImage);
	static void getTextureDataFromFile(const std::string & path, TextureData & textureData);
	static glm::vec2 getImageDimensions(const std::string& path);
	static unsigned int loadTextureFromFile(const std::string& path);
	static unsigned int loadCubemapFromFile(const std::vector<std::string>& paths);
	static unsigned int loadTextureFromData(TextureData & textureData);
	static GLenum getTextureFormatFromData(TextureData & textureData);
	static GLenum getTextureFormatFromData(int componentCount);
	static void SaveImage(const std::string & path, const glm::vec2& imageRes, ImageFormat imageFormat, char * data);
};
