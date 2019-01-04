#pragma once
#include <GL\glew.h>
#include "ColourData.h"
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
	void setTextureData(unsigned char* data, int width, int height, int componentCount);
	unsigned char* getTextureData();
	const int getWidth()noexcept;
	const int getHeight()noexcept;
	int getComponentCount()noexcept;
	void SetTexId(unsigned int texId);
	unsigned int GetTexId();
	void setTexelColor(int r, int g, int b, int a, int x, int y);
	void setTexelColor(ColourData& colourData, int x, int y);
	void updateTexture();
	void updateTextureData(unsigned char* data);
	ColourData getTexelColor(int x, int y);
	void setTextureDirty();

	~TextureData();
};

