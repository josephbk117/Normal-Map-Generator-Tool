#pragma once
enum class WindowSide { NONE = -1, LEFT = 0, TOP_LEFT = 1, TOP = 2, TOP_RIGHT = 3, RIGHT = 4, BOTTOM_RIGHT = 5, BOTTOM = 6, BOTTOM_LEFT = 7, CENTER = 8 };
class WindowTransformUtility
{
public:
	static const int BORDER_SIZE = 5;

	static WindowSide getWindowSideBorderAtMouseCoord(const glm::ivec2 &curPos, const glm::ivec2 &winRes)
	{
		return getWindowSideBorderAtMouseCoord(curPos.x, curPos.y, winRes.x, winRes.y);
	}

	static WindowSide getWindowSideBorderAtMouseCoord(int mouseX, int mouseY, int windowWidth, int windowHeight)
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

	static WindowSide getWindowAreaAtMouseCoord(int mouseX, int mouseY, int windowWidth, int windowHeight)
	{
		float vpMouseX = mouseX / (float)windowWidth;
		float vpMouseY = mouseY / (float)windowHeight;

		const float sideBoundAsPerc = 325.0f / windowWidth;

		if (vpMouseX > sideBoundAsPerc && vpMouseX < 1.0f - sideBoundAsPerc)
		{
			return WindowSide::CENTER;
		}
		else
		{
			if (vpMouseX > 0.0f && vpMouseX <= sideBoundAsPerc)
				return WindowSide::LEFT;
			else if (vpMouseX >= 1.0f - sideBoundAsPerc && vpMouseX <= 1.0f)
				return WindowSide::RIGHT;
		}
		return WindowSide::NONE;
	}
};