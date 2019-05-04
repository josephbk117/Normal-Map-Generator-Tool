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
	void init(const std::string& windowTitle, int windowWidth, int windowHeight);
	void setFrameBufferResizeCallback(void(*func)(GLFWwindow*, int, int));
	void setScrollCallback(void(*func)(GLFWwindow*, double, double));
	bool isKeyPressed(int key)const;
	bool isKeyReleased(int key)const;
	bool isKeyPressedDown(int key)const;
	void destroy();
	void close();
	void updateWindow();
	void minimize();
	const GLFWvidmode* getVideoMode();
	const GLFWwindow* getWindow();
	const int getMinWindowSize()const;
	void setWindowRes(const glm::ivec2 &res);
	void setWindowRes(int windowWidth, int windowHeight);
	void setFullscreen(bool isFullscreen);
	bool getIfFullscreen()const;
	const glm::ivec2 getMaxWindowRes()const;
	const glm::ivec2 getWindowRes()const;
	glm::ivec2 getWindowPos()const;
	glm::ivec2 getCursorPos()const;
	void setWindowPos(int x, int y);
	float getAspectRatio()const;
	bool isWindowClosing()const;
	~WindowSystem();
};

