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

const GLFWvidmode * WindowSystem::GetVideoMode()
{
	return videoMode;
}

const GLFWwindow * WindowSystem::GetWindow()
{
	return window;
}

void WindowSystem::SetWindowRes(int windowWidth, int windowHeight)
{
	glfwSetWindowSize(window, windowWidth, windowHeight);
	windowRes = glm::vec2(windowWidth, windowHeight);
}

const glm::vec2 WindowSystem::GetMaxWindowRes()
{
	return maxWindowRes;
}

const glm::vec2 WindowSystem::GetWindowRes()
{
	return windowRes;
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
