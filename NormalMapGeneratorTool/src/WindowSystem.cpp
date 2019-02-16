#include "WindowSystem.h"
#include <iostream>
//#define NORA_CUSTOM_WINDOW_CHROME
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
	/*glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);*/

	window = glfwCreateWindow(windowRes.x, windowRes.y, windowTitle.c_str(), NULL, NULL);
	videoMode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	maxWindowRes.x = videoMode->width;
	maxWindowRes.y = videoMode->height;
	glfwSetWindowSizeLimits(window, WINDOW_SIZE_MIN, WINDOW_SIZE_MIN, maxWindowRes.x, maxWindowRes.y);
	glfwSetWindowPos(window, 50, 50);

	if (!window)
		return;
	glfwMakeContextCurrent(window);
}

void WindowSystem::SetFrameBufferResizeCallback(void(*func)(GLFWwindow *, int, int))
{
	glfwSetFramebufferSizeCallback(window, func);
}

void WindowSystem::SetScrollCallback(void(*func)(GLFWwindow *, double, double))
{
	glfwSetScrollCallback(window, func);
}

const int WindowSystem::GetMinWindowSize()
{
	return WINDOW_SIZE_MIN;
}

bool WindowSystem::IsKeyPressed(int key)
{
	int state = glfwGetKey(window, key);
	if (state == GLFW_PRESS)
		return true;
	return false;
}

bool WindowSystem::IsKeyReleased(int key)
{
	int state = glfwGetKey(window, key);
	if (state == GLFW_RELEASE)
		return true;
	return false;
}

bool WindowSystem::IsKeyPressedDown(int key)
{
	static int prevKey = GLFW_KEY_0;
	int state = glfwGetKey(window, key);

	if (state == GLFW_PRESS)
	{
		if (prevKey != key)
		{
			prevKey = key;
			return true;
		}
		prevKey = key;
	}
	else if (state == GLFW_RELEASE)
	{
		if (prevKey == key)
			prevKey = GLFW_KEY_0;
	}
	return false;
}

void WindowSystem::Destroy()
{
	glfwDestroyWindow(window);
	glfwTerminate();
}

void WindowSystem::Close()
{
	glfwSetWindowShouldClose(window, true);
}

void WindowSystem::UpdateWindow()
{
	glfwSwapBuffers(window);
	glfwPollEvents();
}

void WindowSystem::Minimize()
{
	glfwIconifyWindow(window);
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
