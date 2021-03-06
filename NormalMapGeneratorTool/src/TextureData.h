#pragma once
#include <GL\glew.h>
#include <GLM\common.hpp>
#include "ColourData.h"
/*Holds raw data about a texture
Resolution data, Channel count and access to individual pixels are provided
*/
class TextureData
{
private:
	unsigned char* data = nullptr;
	int width = 0;
	int height = 0;
	int componentCount = 0;
	unsigned int texId = 0;
	bool requiresUpdate = false;
public:
	TextureData();
	void setTextureDataNonAlloc(unsigned char* data, int width, int height, int componentCount);
	void setTextureData(unsigned char* data, int width, int height, int componentCount);
	unsigned char* const getTextureData()const;
	glm::ivec2 getRes()const noexcept;
	int getComponentCount()const noexcept;
	void setTexId(unsigned int texId);
	unsigned int getTexId()const;
	void setTexelColor(int r, int g, int b, int a, int x, int y);
	void setTexelColor(ColourData& colourData, int x, int y);
	void setTexelRangeWithColour(int beginIndex, int endIndex, ColourData& colourData);
	void updateTexture();
	void updateTextureData(unsigned char* data);
	ColourData getTexelColor(int x, int y)const noexcept;
	ColourData getTexColorAsUV(float x, float y)const noexcept;
	//Set the texture as dirty so that it can be updated
	void setTextureDirty()noexcept;
	void clearRawData();

	~TextureData();
};

