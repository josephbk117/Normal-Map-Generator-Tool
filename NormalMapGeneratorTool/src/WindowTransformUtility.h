#pragma once
enum class WindowSide { NONE = -1, LEFT = 0, TOP_LEFT = 1, TOP = 2, TOP_RIGHT = 3, RIGHT = 4, BOTTOM_RIGHT = 5, BOTTOM = 6, BOTTOM_LEFT = 7 };
class WindowTransformUtility
{
public:
	static const int BORDER_SIZE = 5;

	static WindowSide GetWindowSideAtMouseCoord(int mouseX, int mouseY, int windowWidth, int windowHeight)
	{
		if (mouseX < BORDER_SIZE)
		{
			if (mouseY <= BORDER_SIZE)
				return WindowSide::TOP_LEFT;
			else if (mouseY > windowHeight - BORDER_SIZE)
				return WindowSide::BOTTOM_LEFT;
			return WindowSide::LEFT;
		}
		else if (mouseX > windowWidth - BORDER_SIZE)
		{
			if (mouseY <= BORDER_SIZE)
				return WindowSide::TOP_RIGHT;
			else if (mouseY > windowHeight - BORDER_SIZE)
				return WindowSide::BOTTOM_RIGHT;
			return WindowSide::RIGHT;
		}
		else if (mouseY < BORDER_SIZE)
		{
			return WindowSide::TOP;
		}
		else if (mouseY > windowHeight - BORDER_SIZE)
		{
			return WindowSide::BOTTOM;
		}
		else
			return WindowSide::NONE;
	}
};