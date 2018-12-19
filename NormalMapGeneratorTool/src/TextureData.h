#pragma once
#include "ColourData.h"
class TextureData
{
private:
	unsigned char* data = nullptr;
	int width = 0;
	int height = 0;
	int componentCount = 0;
public:
	TextureData();
	void setTextureData(unsigned char* data, int width, int height, int componentCount);
	unsigned char* getTextureData();
	const int getWidth()noexcept;
	const int getHeight()noexcept;
	int getComponentCount()noexcept;
	void setTexelColor(int r, int g, int b, int a, int x, int y);
	void setTexelColor(ColourData& colourData, int x, int y);
	ColourData getTexelColor(int x, int y);

	~TextureData();
};

