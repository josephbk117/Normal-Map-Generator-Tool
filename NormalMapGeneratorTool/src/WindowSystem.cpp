#include "WindowSystem.h"
#include <iostream>
//#define NORA_CUSTOM_WINDOW_CHROME
WindowSystem::WindowSystem()
{}

void WindowSystem::init(const std::string windowTitle, int windowWidth, int windowHeight)
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

void WindowSystem::setFrameBufferResizeCallback(void(*func)(GLFWwindow *, int, int))
{
	glfwSetFramebufferSizeCallback(window, func);
}

void WindowSystem::setScrollCallback(void(*func)(GLFWwindow *, double, double))
{
	glfwSetScrollCallback(window, func);
}

const int WindowSystem::getMinWindowSize()
{
	return WINDOW_SIZE_MIN;
}

bool WindowSystem::isKeyPressed(int key)
{
	int state = glfwGetKey(window, key);
	if (state == GLFW_PRESS)
		return true;
	return false;
}

bool WindowSystem::isKeyReleased(int key)
{
	int state = glfwGetKey(window, key);
	if (state == GLFW_RELEASE)
		return true;
	return false;
}

bool WindowSystem::isKeyPressedDown(int key)
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

void WindowSystem::destroy()
{
	glfwDestroyWindow(window);
	glfwTerminate();
}

void WindowSystem::close()
{
	glfwSetWindowShouldClose(window, true);
}

void WindowSystem::updateWindow()
{
	glfwSwapBuffers(window);
	glfwPollEvents();
}

void WindowSystem::minimize()
{
	glfwIconifyWindow(window);
}

const GLFWvidmode * WindowSystem::getVideoMode()
{
	return videoMode;
}

const GLFWwindow * WindowSystem::getWindow()
{
	return window;
}

void WindowSystem::setWindowRes(const glm::vec2 & res)
{
	setWindowRes(res.x, res.y);
}

void WindowSystem::setWindowRes(int windowWidth, int windowHeight)
{
	glfwSetWindowSize(window, windowWidth, windowHeight);
	windowRes = glm::vec2(windowWidth, windowHeight);
}

void WindowSystem::setFullscreen(bool isFullscreen)
{
	if (isFullscreen)
		glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, windowRes.x, windowRes.y, 60);
	else
		glfwSetWindowMonitor(window, NULL, 100, 100, (windowRes.x / 1.2f), (windowRes.y / 1.2f), 60);
	this->isFullscreen = isFullscreen;
}

bool WindowSystem::getIfFullscreen()
{
	return isFullscreen;
}

const glm::vec2 WindowSystem::getMaxWindowRes()
{
	return maxWindowRes;
}

const glm::vec2 WindowSystem::getWindowRes()
{
	return windowRes;
}

glm::vec2 WindowSystem::getWindowPos()
{
	int winPosX, winPosY;
	glfwGetWindowPos(window, &winPosX, &winPosY);
	return glm::vec2(winPosX, winPosY);
}

glm::vec2 WindowSystem::getCursorPos()
{
	double x, y;
	glfwGetCursorPos(window, &x, &y);
	return glm::vec2(x, y);
}

void WindowSystem::setWindowPos(int x, int y)
{
	glfwSetWindowPos(window, x, y);
}

float WindowSystem::getAspectRatio()
{
	return (float)windowRes.x / (float)windowRes.y;
}

bool WindowSystem::isWindowClosing()
{
	return glfwWindowShouldClose(window);
}

WindowSystem::~WindowSystem()
{
}
