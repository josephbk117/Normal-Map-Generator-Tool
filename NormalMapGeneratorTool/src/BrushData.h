#pragma once
struct BrushData
{
	float brushScale = 0.05f;
	float brushOffset = 0.01f;
	float brushStrength = 1.0f;
	float brushMinHeight = 0.0f;
	float brushMaxHeight = 1.0f;
	float brushRate = 0.0f;
	bool heightMapPositiveDir = false;

	bool operator!= (const BrushData &bD)
	{
		if (brushScale != bD.brushScale || brushOffset != bD.brushOffset ||
			brushStrength != bD.brushStrength || brushMinHeight != bD.brushMinHeight ||
			brushMaxHeight != bD.brushMaxHeight || brushRate != bD.brushRate || heightMapPositiveDir != bD.heightMapPositiveDir)
		{
			return true;
		}
		return false;
	}
};