#pragma once
#include <GLM\common.hpp>
struct ColourData
{
private:
	glm::vec4 colour;
public:
	//Default white
	ColourData() noexcept
	{
		colour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	}
	//Takes input in 32 bit space
	ColourData(float r, float g, float b, float a)
	{
		colour = glm::vec4(r, g, b, a);
	}
	void setColour_8_Bit(const glm::vec4& colour)
	{
		const int r = glm::clamp(static_cast<int>(colour.r), 0, 255);
		const int g = glm::clamp(static_cast<int>(colour.g), 0, 255);
		const int b = glm::clamp(static_cast<int>(colour.b), 0, 255);
		const int a = glm::clamp(static_cast<int>(colour.a), 0, 255);
		this->colour = glm::vec4(r, g, b, a);
	}
	void setColour_8_Bit(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
	{
		const float _r = r / 255.0f;
		const float _g = g / 255.0f;
		const float _b = b / 255.0f;
		const float _a = a / 255.0f;
		this->colour = glm::vec4(_r, _g, _b, _a);
	}
	void setColour_32_bit(const glm::vec4& colour)noexcept
	{
		this->colour = colour;
	}
	void setColour_32_bit(float r, float g, float b, float a)
	{
		this->colour = glm::vec4(r, g, b, a);
	}
	const glm::vec4 getColour_32_Bit() const noexcept
	{
		return colour;
	}
	const glm::vec4 getColour_8_Bit() const
	{
		const unsigned char rV = static_cast<unsigned char>(colour.r * 255.0f);
		const unsigned char gV = static_cast<unsigned char>(colour.g * 255.0f);
		const unsigned char bV = static_cast<unsigned char>(colour.b * 255.0f);
		const unsigned char aV = static_cast<unsigned char>(colour.a * 255.0f);
		return glm::vec4(rV, gV, bV, aV);
	}
};