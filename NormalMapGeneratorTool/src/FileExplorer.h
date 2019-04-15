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
	static FileExplorer* instance;
	bool isDirty = true;
	bool shouldDisplay = false;
	std::string path = "C:\\";
	static void init();
	static void shutDown();
	void display();
	void displayDialog( FileType filter = FileType::NONE) noexcept;
	void displayDialog( FileType filter, std::function<void(std::string)> func) noexcept;
	bool doesPathExist(const std::string& path)noexcept;
	std::string getFileExtension(const std::string & path);
	std::vector<std::string> getAllFilesInDirectory(const std::string& path, bool withEntirePath, const std::string& fileExt = "");
	unsigned long getFileSize(const std::string path)const;
private:
	std::vector<std::string> paths;
	std::vector<std::string> roots;
	std::function<void(std::string)> functionToCall = nullptr;
	FileType fileFilter = FileType::NONE;
	bool pathTypeCheck(std::vector<std::string> endTypes, std::string& _path);
	FileExplorer();
	~FileExplorer();
};