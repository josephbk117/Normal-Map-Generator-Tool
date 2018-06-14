#include "TextureData.h"
TextureData::TextureData()
{
}

void TextureData::setTextureData(unsigned char * data, int width, int height, int componentCount)
{
	this->data = data;
	this->width = width;
	this->height = height;
	this->componentCount = componentCount;
}

unsigned char * TextureData::getTextureData()
{
	return data;
}

int TextureData::getWidth()
{
	return width;
}

int TextureData::getHeight()
{
	return height;
}

int TextureData::getComponentCount()
{
	return componentCount;
}

void TextureData::setTexelColor(unsigned char r, unsigned char g, unsigned char b, int x, int y)
{
	int i = ((float)width * (float)y + (float)x) * 4.0f;
	data[i] = r;
	data[i + 1] = g;
	data[i + 2] = b;
}

TextureData::~TextureData()
{
	delete[] data;
}
