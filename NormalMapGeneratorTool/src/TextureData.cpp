#include "TextureData.h"
#include "TextureLoader.h"
TextureData::TextureData()
{
	data = nullptr;
	width = 0;
	height = 0;
	componentCount = 0;
}

void TextureData::setTextureData(unsigned char * data, int width, int height, int componentCount)
{
	this->width = width;
	this->height = height;
	this->componentCount = componentCount;
	if (this->data != nullptr)
	{
		delete[] this->data;
		this->data = nullptr;
	}
	this->data = new unsigned char[width * height * componentCount];
	std::memcpy(this->data, data, width * height * componentCount);
}

unsigned char * TextureData::getTextureData()
{
	return data;
}

glm::vec2 TextureData::getRes() noexcept
{
	return glm::vec2(width, height);
}

int TextureData::getComponentCount() noexcept
{
	return componentCount;
}

void TextureData::SetTexId(unsigned int texId)
{
	if (this->texId != 0)
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
	glm::vec4 colour = colourData.getColour_8_Bit();
	data[i] = colour.r;
	data[i + 1] = colour.g;
	data[i + 2] = colour.b;
	data[i + 3] = colour.a;
}

void TextureData::setTexelRangeWithColour(int beginIndex, int endIndex, ColourData & colourData)
{
	int count = beginIndex;
	glm::vec4 colour = colourData.getColour_8_Bit();
	for (int i = beginIndex; i < endIndex; i++)
	{
		data[i] = colour.r;
		data[i + 1] = colour.g;
		data[i + 2] = colour.b;
		data[i + 3] = colour.a;
		count += 4;
	}
}

void TextureData::updateTexture()
{
	if (!requiresUpdate)
		return;
	GLint currentTexture;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &currentTexture);
	GLenum format = TextureManager::getTextureFormatFromData(componentCount);
	glBindTexture(GL_TEXTURE_2D, texId);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, data); //Issue is here 
	glBindTexture(GL_TEXTURE_2D, currentTexture);
}

void TextureData::updateTextureData(unsigned char * data)
{
	std::memcpy(this->data, data, width * height * componentCount);
}

ColourData TextureData::getTexelColor(int x, int y)noexcept
{
	int i = ((float)width * (float)y + (float)x) * componentCount;
	ColourData colData;
	colData.setColour_8_Bit(data[i], data[i + 1], data[i + 2], data[i + 3]);
	return colData;
}

void TextureData::setTextureDirty()noexcept
{
	requiresUpdate = true;
}

TextureData::~TextureData()
{
	if (texId != 0)
		glDeleteTextures(1, &texId);
	delete[] data;
}
