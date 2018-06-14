#pragma once
#include <GLM\common.hpp>
struct ColourData
{
private:
	glm::vec4 colour;
public:
	void setColour_8_Bit(const glm::vec4& colour)
	{
		int r = glm::clamp((int)colour.r, 0, 255);
		int g = glm::clamp((int)colour.g, 0, 255);
		int b = glm::clamp((int)colour.b, 0, 255);
		int a = glm::clamp((int)colour.a, 0, 255);
		this->colour = glm::vec4(r, g, b, a);
	}
	void setColour(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
	{
		this->colour = glm::vec4(r, g, b, a);
	}
	glm::vec4 getColourIn_0_1_Range()
	{
		return glm::vec4(colour.r / 255.0f, colour.g / 255.0f, colour.b / 255.0f, colour.a / 255.0f);
	}
	glm::vec4 getColour_8_Bit()
	{
		return colour;
	}
};