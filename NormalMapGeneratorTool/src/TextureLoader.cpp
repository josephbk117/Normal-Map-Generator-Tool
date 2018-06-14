#include "TextureLoader.h"
#include "stb_image.h"
#include "TextureData.h"
#include <GL\glew.h>
#include <iostream>

std::unordered_map<std::string, unsigned int> TextureManager::textureIdMap;

void TextureManager::getRawImageDataFromFile(const std::string & path, std::vector<unsigned char>& data, int &width, int &height, bool flipImage)
{
	int nrComponents;
	stbi_set_flip_vertically_on_load(flipImage);
	unsigned char* ldata = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
	unsigned int size = width * height * nrComponents;
	for (unsigned int i = 0; i < size; i++)
		data.push_back(ldata[i]);
}

void TextureManager::getTextureDataFromFile(const std::string & path, TextureData & textureData)
{
	int width, height, nrComponents;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
	if (data)
		textureData.setTextureData(data, width, height, nrComponents);
}

glm::vec2 TextureManager::getImageDimensions(const std::string & path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	int width, height, nrComponents;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
	free(data);
	return glm::vec2(width, height);
}

unsigned int TextureManager::loadTextureFromFile(const std::string & path, const std::string& referenceString, bool gamma)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	int width, height, nrComponents;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format = GL_RED;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;
		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	}
#if _DEBUG
	else
		std::cout << "\nTexture failed to load at path: " << path.c_str() << std::endl;
#endif
	stbi_image_free(data);
	textureIdMap[referenceString] = textureID;
	return textureID;
}

unsigned int TextureManager::loadTextureFromData(TextureData & textureData, bool gamma)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	unsigned char* data = textureData.getTextureData();
	if (data)
	{
		GLenum format = GL_RED;
		if (textureData.getComponentCount() == 1)
			format = GL_RED;
		else if (textureData.getComponentCount() == 3)
			format = GL_RGB;
		else if (textureData.getComponentCount() == 4)
			format = GL_RGBA;
		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, textureData.getWidth(), textureData.getHeight(), 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	}
#if _DEBUG
	else
		std::cout << "\nTexture failed to load with texture data "<< std::endl;
#endif
	return textureID;
}

unsigned int TextureManager::getTextureIdFromReference(const std::string& referenceString)
{
	return textureIdMap[referenceString];
}

void TextureManager::unloadTexture(const std::string & referenceName)
{
	glDeleteTextures(1, &textureIdMap[referenceName]);
	textureIdMap.erase(referenceName);
}

void TextureManager::unloadTexturesFromMemory()
{
	textureIdMap.erase(textureIdMap.begin(), textureIdMap.end());
}
