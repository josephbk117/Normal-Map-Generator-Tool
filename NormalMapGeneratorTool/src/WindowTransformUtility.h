#pragma once
enum class WindowSide { NONE = -1, LEFT = 0, TOP_LEFT = 1, TOP = 2, TOP_RIGHT = 3, RIGHT = 4, BOTTOM_RIGHT = 5, BOTTOM = 6, BOTTOM_LEFT = 7 };
static class WindowTransformUtility
{
public:
	static WindowSide GetWindowSideAtMouseCoord(int mouseX, int mouseY, int windowWidth, int windowHeight)
	{
		if (mouseX < 15)
		{
			if (mouseY <= 15)
				return WindowSide::TOP_LEFT;
			else if (mouseY > windowHeight - 15)
				return WindowSide::BOTTOM_LEFT;
			return WindowSide::LEFT;
		}
		else if (mouseX > windowWidth - 15)
		{
			if (mouseY <= 15)
				return WindowSide::TOP_RIGHT;
			else if (mouseY > windowHeight - 15)
				return WindowSide::BOTTOM_RIGHT;
			return WindowSide::RIGHT;
		}
		else if (mouseY < 15)
		{
			return WindowSide::TOP;
		}
		else if (mouseY > windowHeight - 15)
		{
			return WindowSide::BOTTOM;
		}
		else
			return WindowSide::NONE;
	}
};