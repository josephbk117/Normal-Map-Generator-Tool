#pragma once
#include "TextureData.h"
#include "GLutil.h"
#include <GL\glew.h>
#include <unordered_map>
#include <vector>
#include <GLM\common.hpp>
enum class ImageFormat { PNG = 100, JPEG, BMP, TGA, UNCOMPRESSED_RAW };

class TextureManager
{
public:
	TextureManager() = delete;
	TextureManager(const TextureManager&) = delete;
	static void getRawImageDataFromFile(const std::string& path, std::vector<unsigned char>& data, int &width, int &height, bool flipImage);
	static void getTextureDataFromFile(const std::string & path, TextureData & textureData);
	static glm::ivec2 getImageDimensions(const std::string& path);
	static unsigned int createTextureFromFile(const std::string& path, bool linearColourSpace = false, TextureFilterType textureFilterType = TextureFilterType::LINEAR)noexcept;
	static unsigned int createCubemapFromFile(const std::vector<std::string>& paths);
	static unsigned int createTextureFromData(const TextureData & textureData, TextureFilterType textureFilterType = TextureFilterType::LINEAR);
	static GLenum getTextureFormatFromData(const TextureData & textureData)noexcept;
	static GLenum getTextureFormatFromData(int componentCount)noexcept;
	static void saveImage(const std::string& path, const glm::ivec2& imageRes, ImageFormat imageFormat, char * data);
	static unsigned int createTextureFromColour(const ColourData& colour, TextureFilterType textureFilterType);
};
