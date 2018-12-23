#pragma once
#include <vector>
#include <functional>
enum class FileType
{
	IMAGE, TEXT, MODEL, NONE
};

class FileExplorer
{
public:
	static FileExplorer instance;
	bool isDirty = true;
	bool shouldDisplay = false;
	std::string path = "C:\\";
	FileExplorer();
	void display();
	void displayDialog( FileType filter = FileType::NONE) noexcept;
	void displayDialog( FileType filter, std::function<void(std::string)> func) noexcept;
	~FileExplorer();
private:
	std::vector<std::string> paths;
	std::vector<std::string> roots;
	std::function<void(std::string)> functionToCall = nullptr;
	FileType fileFilter = FileType::NONE;
	bool pathTypeCheck(std::vector<std::string> endTypes, std::string& _path);
};