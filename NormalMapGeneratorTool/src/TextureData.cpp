#include "TextureData.h"
TextureData::TextureData()
{
	data = nullptr;
	width = 0;
	height = 0;
	componentCount = 0;
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

const int TextureData::getWidth() noexcept
{
	return width;
}

const int TextureData::getHeight() noexcept
{
	return height;
}

int TextureData::getComponentCount() noexcept
{
	return componentCount;
}

void TextureData::SetTexId(unsigned int texId)
{
	if(this->texId != 0)
		glDeleteTextures(1, &(this->texId));
	this->texId = texId;
}

unsigned int TextureData::GetTexId()
{
	return texId;
}

void TextureData::setTexelColor(int r, int g, int b, int a, int x, int y)
{
	x = glm::clamp(x, 0, width);
	y = glm::clamp(y, 0, height);
	int i = ((float)width * (float)y + (float)x) * 4.0f;
	r = glm::clamp(r, 0, 255);
	g = glm::clamp(g, 0, 255);
	b = glm::clamp(b, 0, 255);
	a = glm::clamp(a, 0, 255);

	data[i] = r;
	data[i + 1] = g;
	data[i + 2] = b;
	data[i + 3] = a;
}

void TextureData::setTexelColor(ColourData & colourData, int x, int y)
{
	x = glm::clamp(x, 0, width);
	y = glm::clamp(y, 0, height);
	int i = ((float)width * (float)y + (float)x) * 4.0f;
	data[i] = colourData.getColour_8_Bit().r;
	data[i + 1] = colourData.getColour_8_Bit().g;
	data[i + 2] = colourData.getColour_8_Bit().b;
	data[i + 3] = colourData.getColour_8_Bit().a;
}

ColourData TextureData::getTexelColor(int x, int y)
{
	int i = ((float)width * (float)y + (float)x) * 4.0f;
	ColourData colData;
	colData.setColour_8_Bit(data[i], data[i + 1], data[i + 2], data[i + 3]);
	return colData;
}

TextureData::~TextureData()
{
	if(texId != 0)
		glDeleteTextures(1, &texId);
	delete[] data;
}
