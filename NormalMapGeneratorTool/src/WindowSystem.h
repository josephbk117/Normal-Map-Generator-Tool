#pragma once
#include <GLFW/glfw3.h>
#include <GLM\common.hpp>
#include <string>
class WindowSystem
{
private:
	const GLFWvidmode * videoMode;
	const int WINDOW_SIZE_MIN = 640;
	GLFWwindow * window;
	//Temporary Hack, Max resoltion will not face issues
	std::string windowTitle="NIL";
	glm::vec2 windowRes = glm::vec2(1600, 800);
	glm::vec2 maxWindowRes = glm::vec2(-1, -1);
public:
	WindowSystem();
	void Init(const std::string windowTitle, int windowWidth, int windowHeight);
	const GLFWvidmode* GetVideoMode();
	void SetWindowRes(int windowWidth, int windowHeight);
	const glm::vec2 GetMaxWindowRes();
	const glm::vec2 GetWindowRes();
	float GetAspectRatio();
	~WindowSystem();
};

