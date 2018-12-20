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
	void Init(const std::string windowTitle, int windowWidth, int windowHeight);
	void Destroy();
	void Close();
	void UpdateWindow();
	void Minimize();
	const GLFWvidmode* GetVideoMode();
	const GLFWwindow* GetWindow();
	void SetWindowRes(const glm::vec2 &res);
	void SetWindowRes(int windowWidth, int windowHeight);
	void SetFullscreen(bool isFullscreen);
	bool IsFullscreen();
	const glm::vec2 GetMaxWindowRes();
	const glm::vec2 GetWindowRes();
	glm::vec2 GetWindowPos();
	glm::vec2 GetCursorPos();
	void SetWindowPos(int x, int y);
	float GetAspectRatio();
	bool IsWindowClosing();
	~WindowSystem();
};

