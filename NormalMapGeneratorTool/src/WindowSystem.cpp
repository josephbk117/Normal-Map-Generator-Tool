#include "WindowSystem.h"

WindowSystem::WindowSystem()
{}

void WindowSystem::Init(const std::string windowTitle, int windowWidth, int windowHeight)
{
	this->windowTitle = windowTitle;
	windowRes.x = windowWidth;
	windowRes.y = windowHeight;
	if (!glfwInit())
		return;
#ifdef NORA_CUSTOM_WINDOW_CHROME
	glfwWindowHint(GLFW_DECORATED, false);
#endif
	window = glfwCreateWindow(windowRes.x, windowRes.y, "Nora Normal Map Editor v0.8 alpha", NULL, NULL);
	videoMode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	maxWindowRes.x = videoMode->width;
	maxWindowRes.y = videoMode->height;
	glfwSetWindowSizeLimits(window, WINDOW_SIZE_MIN, WINDOW_SIZE_MIN, maxWindowRes.x, maxWindowRes.y);
	glfwSetWindowPos(window, 0, 0);

	if (!window)
	{
		glfwTerminate();
		return;
	}
	glfwMakeContextCurrent(window);
}

void WindowSystem::Close()
{
	glfwDestroyWindow(window);
	glfwTerminate();
}

void WindowSystem::UpdateWindow()
{
	glfwSwapBuffers(window);
	glfwPollEvents();
}

const GLFWvidmode * WindowSystem::GetVideoMode()
{
	return videoMode;
}

const GLFWwindow * WindowSystem::GetWindow()
{
	return window;
}

void WindowSystem::SetWindowRes(const glm::vec2 & res)
{
	SetWindowRes(res.x, res.y);
}

void WindowSystem::SetWindowRes(int windowWidth, int windowHeight)
{
	glfwSetWindowSize(window, windowWidth, windowHeight);
	windowRes = glm::vec2(windowWidth, windowHeight);
}

void WindowSystem::SetFullscreen(bool isFullscreen)
{
	if (isFullscreen)
		glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, windowRes.x, windowRes.y, 60);
	else
		glfwSetWindowMonitor(window, NULL, 100, 100, (windowRes.x / 1.2f), (windowRes.y / 1.2f), 60);
	this->isFullscreen = isFullscreen;
}

bool WindowSystem::IsFullscreen()
{
	return isFullscreen;
}

const glm::vec2 WindowSystem::GetMaxWindowRes()
{
	return maxWindowRes;
}

const glm::vec2 WindowSystem::GetWindowRes()
{
	return windowRes;
}

glm::vec2 WindowSystem::GetWindowPos()
{
	int winPosX, winPosY;
	glfwGetWindowPos(window, &winPosX, &winPosY);
	return glm::vec2(winPosX, winPosY);
}

glm::vec2 WindowSystem::GetCursorPos()
{
	double x, y;
	glfwGetCursorPos(window, &x, &y);
	return glm::vec2(x, y);
}

void WindowSystem::SetWindowPos(int x, int y)
{
	glfwSetWindowPos(window, x, y);
}

float WindowSystem::GetAspectRatio()
{
	return (float)windowRes.x / (float)windowRes.y;
}

bool WindowSystem::IsWindowClosing()
{
	return glfwWindowShouldClose(window);
}

WindowSystem::~WindowSystem()
{
}
