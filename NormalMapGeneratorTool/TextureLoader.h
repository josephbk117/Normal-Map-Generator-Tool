#pragma once
#include <unordered_map>
#include <vector>
#include <GLM\common.hpp>
class TextureManager
{
public:
	TextureManager() = delete;
	TextureManager(const TextureManager&) = delete;
	static void getRawImageDataFromFile(const std::string& path, std::vector<unsigned char>& data, int &width, int &height, bool flipImage);
	static glm::vec2 getImageDimensions(const std::string& path);
	static unsigned int loadTextureFromFile(const std::string& path, const std::string& referenceName, bool gamma);
	static unsigned int getTextureIdFromReference(const std::string& referenceName);
	static void unloadTexture(const std::string& referenceName);
	static void unloadTexturesFromMemory();
private:
	static std::unordered_map<std::string, unsigned int> textureIdMap;
};