#pragma once
#include <string>
#include "imgui.h"
class ModalWindow
{
private:
	std::string title = "";
	std::string content = "";
	ImFont* titleFont = NULL;
	bool shouldShow = false;
public:
	ModalWindow();
	void setTitleFont(ImFont* menuBarLargerText);
	void setModalDialog(const std::string& title, const std::string& content);
	void display();
	~ModalWindow();
};

