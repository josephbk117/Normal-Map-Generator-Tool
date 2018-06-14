#pragma once
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
	~TextureData();
};

