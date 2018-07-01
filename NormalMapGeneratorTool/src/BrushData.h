#pragma once
struct BrushData
{
	float brushScale = 0.05f;
	float brushOffset = 1.0f;
	float brushStrength = 1.0f;
	float brushMinHeight = 0.0f;
	float brushMaxHeight = 1.0f;
	float brushRate = 0.0f;
	bool heightMapPositiveDir = false;
};