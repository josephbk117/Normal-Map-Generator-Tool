#pragma once
#include "TextureData.h"
struct BrushData
{
	float brushScale = 25.0f;
	float brushOffset = 0.25f;
	float brushStrength = 1.0f;
	float brushMinHeight = 0.0f;
	float brushMaxHeight = 1.0f;
	float brushRate = 0.0f;
	bool heightMapPositiveDir = false;
	TextureData textureData;

	bool operator!= (const BrushData &bD)
	{
		if (brushScale != bD.brushScale || brushOffset != bD.brushOffset ||
			brushStrength != bD.brushStrength || brushMinHeight != bD.brushMinHeight ||
			brushMaxHeight != bD.brushMaxHeight || brushRate != bD.brushRate ||
			heightMapPositiveDir != bD.heightMapPositiveDir || &textureData != &bD.textureData)
		{
			return true;
		}
		return false;
	}

	bool hasBrushTexture()const noexcept
	{
		return textureData.getTextureData() != nullptr;
	}
};