#pragma once
#include <GLFW/glfw3.h>
#include <GLM\common.hpp>
#include <string>
class WindowSystem
{
/*Define For Enabling Custom Window Chrome*/
//#define NORA_CUSTOM_WINDOW_CHROME
private:
	const int WINDOW_SIZE_MIN = 640;

	const GLFWvidmode * videoMode = nullptr;
	GLFWwindow * window = nullptr;
	bool isFullscreen = false;
	std::string windowTitle="NIL";
	glm::vec2 windowRes = glm::vec2(1600, 800);
	glm::vec2 maxWindowRes = glm::vec2(-1, -1);
public:
	WindowSystem();
	void init(const std::string windowTitle, int windowWidth, int windowHeight);
	void setFrameBufferResizeCallback(void(*func)(GLFWwindow*, int, int));
	void setScrollCallback(void(*func)(GLFWwindow*, double, double));
	bool isKeyPressed(int key);
	bool isKeyReleased(int key);
	bool isKeyPressedDown(int key);
	void destroy();
	void close();
	void updateWindow();
	void minimize();
	const GLFWvidmode* getVideoMode();
	const GLFWwindow* getWindow();
	const int getMinWindowSize();
	void setWindowRes(const glm::vec2 &res);
	void setWindowRes(int windowWidth, int windowHeight);
	void setFullscreen(bool isFullscreen);
	bool getIfFullscreen();
	const glm::vec2 getMaxWindowRes();
	const glm::vec2 getWindowRes();
	glm::vec2 getWindowPos();
	glm::vec2 getCursorPos();
	void setWindowPos(int x, int y);
	float getAspectRatio();
	bool isWindowClosing();
	~WindowSystem();
};

