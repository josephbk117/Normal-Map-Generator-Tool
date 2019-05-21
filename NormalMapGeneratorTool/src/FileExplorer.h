#pragma once
#include <vector>
#include <functional>
enum class FileType
{
	IMAGE, TEXT, MODEL, NORA, NONE
};

class FileExplorer
{
public:
	virtual void display() = 0;
	bool doesPathExist(const std::string& path)const noexcept;
	std::string getFileExtension(const std::string & path)const;
	std::vector<std::string> getAllFilesInDirectory(const std::string& path, bool withEntirePath, const std::string& fileExt = "")const;
	unsigned long getFileSize(const std::string path)const;
};

class FileOpenDialog : public FileExplorer
{
public:
	static FileOpenDialog* instance;
	bool isDirty = true;
	bool shouldDisplay = false;
	std::string path = "C:\\";
	static void init();
	static void shutDown();
	void display() override;
	void displayDialog( FileType filter = FileType::NONE) noexcept;
	void displayDialog( FileType filter, std::function<void(std::string)> func) noexcept;
private:
	std::vector<std::string> paths;
	std::vector<std::string> roots;
	std::function<void(std::string)> functionToCall = nullptr;
	FileType fileFilter = FileType::NONE;
	bool pathTypeCheck(std::vector<std::string> endTypes, std::string& _path);
};

class FileSaveDialog : public FileExplorer
{
public:
	static FileSaveDialog* instance;
	bool isDirty = true;
	bool shouldDisplay = false;
	std::string path = "C:\\";
	static void init();
	static void shutDown();
	void display() override;
	void displayDialog(FileType filter, std::function<void(std::string)> func) noexcept;
private:
	std::vector<std::string> paths;
	std::vector<std::string> roots;
	std::function<void(std::string)> functionToCall = nullptr;
	FileType fileFilter = FileType::NONE;
};