#pragma once
#include "ColourData.h"
class TextureData
{
private:
	unsigned char* data;
	int width;
	int height;
	int componentCount;
public:
	TextureData();
	void setTextureData(unsigned char* data, int width, int height, int componentCount);
	unsigned char* getTextureData();
	int getWidth();
	int getHeight();
	int getComponentCount();
	void setTexelColor(unsigned char r, unsigned char g, unsigned char b, int x, int y);
	void setTexelColor(ColourData& colourData, int x, int y);
	ColourData getTexelColor(int x, int y);

	~TextureData();
};

