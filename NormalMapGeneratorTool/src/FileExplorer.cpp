#include "FileExplorer.h"
#include "ImGui\imgui.h"
#include <vector>
#include <filesystem>
#include <iostream>
#include <string>
FileExplorer *FileExplorer::instance = nullptr;
FileExplorer::FileExplorer()
{
}

void FileExplorer::init()
{
	if (instance == nullptr)
		instance = new FileExplorer();
	std::string rootPath = "C:\\";
	for (unsigned int i = 0; i < 5; i++)
	{
		if (std::experimental::filesystem::exists(rootPath))
			instance->roots.push_back(rootPath);
		rootPath[0] = (char)((int)rootPath[0] + 1);
	}
	for (int i = 0; i < instance->roots.size(); i++)
		std::cout << "\nPATH : " << instance->roots[i];
}

void FileExplorer::shutDown()
{
	if (instance != nullptr)
		delete instance;
}

void FileExplorer::display()
{
	if (!shouldDisplay)
		return;

	static char pathInput[500];
	if (isDirty)
	{
		paths.clear();
		for (int i = 0; i < path.length(); i++)
		{
			pathInput[i] = path[i];
		}
		std::vector<std::string> filterEnd;
		switch (fileFilter)
		{
		case FileType::IMAGE:
			filterEnd.push_back(".png"); filterEnd.push_back(".jpg"); filterEnd.push_back(".bmp"); filterEnd.push_back(".psd");
			break;
		case FileType::TEXT:
			filterEnd.push_back(".txt");
			break;
		case FileType::MODEL:
			filterEnd.push_back(".obj"); filterEnd.push_back(".fbx");
			break;
		case FileType::NONE:
		default:
			break;
		}

		for (auto & p : std::experimental::filesystem::directory_iterator(path))
		{
			if (std::experimental::filesystem::is_regular_file(p))
			{
				bool isCorrectFileType = false;
				if (fileFilter != FileType::NONE)
				{
					for (int i = 0; i < filterEnd.size(); i++)
					{
						if (getFileExtension(p.path().generic_string()) == filterEnd[i])
						{
							isCorrectFileType = true;
							break;
						}
					}
				}
				else
					isCorrectFileType = true;
				if (isCorrectFileType)
					paths.push_back(p.path().generic_string());
			}
			else if (std::experimental::filesystem::is_directory(p))
				paths.push_back(p.path().generic_string());
		}
		isDirty = false;
	}
	ImGui::OpenPopup("File Explorer");
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar;
	ImGui::SetNextWindowSizeConstraints(ImVec2(540, 540), ImVec2(1920, 1080));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
	if (ImGui::BeginPopupModal("File Explorer", NULL, window_flags))
	{
		if (ImGui::Button("CLOSE", ImVec2(80, 30)))
		{
			shouldDisplay = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		for (unsigned int i = 0; i < roots.size(); i++)
		{
			ImGui::SameLine();
			if (ImGui::Button(roots[i].c_str(), ImVec2(30, 30))) { path = roots[i]; isDirty = true; }
		}

		ImGui::SameLine();
		ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth() - 40);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 7));
		ImGui::InputText("##Path Field", pathInput, 500);
		ImGui::PopStyleVar();
		ImGui::PopItemWidth();
		ImGui::SameLine();
		if (ImGui::Button("GO", ImVec2(30, 30)))
		{
			path = std::string(pathInput);
			isDirty = true;
		}
		ImGui::Spacing();
		static ImGuiTextFilter filter;
		filter.Draw("Filter(inc, -exc)", ImGui::GetContentRegionAvailWidth() - 200);
		static int columnCount = 1;
		ImGui::SameLine();
		ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvailWidth() - 90, 0));
		ImGui::PushItemWidth(80);
		ImGui::SameLine();
		ImGui::PushStyleVar(ImGuiStyleVar_GrabRounding, 5.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
		ImGui::SliderInt("##Columns", &columnCount, 1, 5);
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("Column count");
		ImGui::PopStyleVar();
		ImGui::PopStyleVar();
		ImGui::PopItemWidth();
		ImGui::Spacing();
		if (!std::experimental::filesystem::is_empty(path))
		{
			ImGui::BeginChild("directory_files", ImVec2(ImGui::GetWindowContentRegionWidth(), ImGui::GetFrameHeight() - 50), true, ImGuiWindowFlags_HorizontalScrollbar);
			for (int i = 0; i < paths.size(); i++)
			{
				std::string strPath = paths[i];

				if (filter.PassFilter(strPath.c_str()))
				{
					bool canShow = true;
					float width = (ImGui::GetContentRegionAvailWidth() * 1.0f / columnCount) - 5;
					if (i % columnCount != 0)
						ImGui::SameLine();
					if (ImGui::Button(strPath.c_str(), ImVec2(width, 30)))
					{
						//if (!pathTypeCheck(filterEnd, strPath))
						isDirty = true;
						path = strPath;
					}
				}
			}
			ImGui::EndChild();
		}
		else
		{
			std::cout << "\nIs empty " << path;
			const int locationOfLastSlash = path.find_last_of('//');
			const int locationOfFirstSlash = path.find_first_of('//');

			if (locationOfFirstSlash <= locationOfLastSlash && path.length() > 3)
			{
				path = path.substr(0, locationOfLastSlash);
				if (path.length() < 3)
					path += '//';
				isDirty = true;
			}
		}
		ImGui::Spacing();
		if (ImGui::Button("BACK"))
		{
			const int locationOfLastSlash = path.find_last_of('//');
			const int locationOfFirstSlash = path.find_first_of('//');

			if (locationOfFirstSlash <= locationOfLastSlash && path.length() > 3)
			{
				path = path.substr(0, locationOfLastSlash);
				if (path.length() < 3)
					path += '//';
				isDirty = true;
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("SELECT"))
		{
			if (functionToCall != nullptr)
			{
				functionToCall(path);
			}
			shouldDisplay = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::PopStyleVar();
		ImGui::SameLine();
		ImGui::Text(path.c_str());
		ImGui::EndPopup();
	}
}

void FileExplorer::displayDialog(FileType filter) noexcept
{
	shouldDisplay = true;
	this->fileFilter = filter;
}

void FileExplorer::displayDialog(FileType filter, std::function<void(std::string)> func) noexcept
{
	shouldDisplay = true;
	isDirty = true;
	this->fileFilter = filter;
	functionToCall = func;
}

bool FileExplorer::doesPathExist(const std::string & path) noexcept
{
	return std::experimental::filesystem::exists(path);
}

std::string FileExplorer::getFileExtension(const std::string & path)
{
	// Create a Path object from given string
	std::experimental::filesystem::path pathObj(path);
	// Check if file name in the path object has extension
	if (pathObj.has_extension())// Fetch the extension from path object and return
		return pathObj.extension().string();
	// In case of no extension return empty string
	return "";
}

std::vector<std::string> FileExplorer::getAllFilesInDirectory(const std::string & path, bool withEntirePath, const std::string& fileExt)
{
	std::vector<std::string> paths;
	if (withEntirePath)
	{
		for (auto & p : std::experimental::filesystem::directory_iterator(path))
		{
			if (std::experimental::filesystem::is_regular_file(p))
			{
				if (fileExt == "")
					paths.push_back(p.path().generic_string());
				else
				{
					if (getFileExtension(p.path().filename().string()) == fileExt)
						paths.push_back(p.path().string());
				}
			}
		}
	}
	else
	{
		for (auto & p : std::experimental::filesystem::directory_iterator(path))
		{
			if (std::experimental::filesystem::is_regular_file(p))
			{
				if (fileExt == "")
					paths.push_back(p.path().filename().string());
				else
				{
					if (getFileExtension(p.path().filename().string()) == fileExt)
						paths.push_back(p.path().filename().string());
				}
			}
		}
	}
	return paths;
}

FileExplorer::~FileExplorer()
{}

bool FileExplorer::pathTypeCheck(std::vector<std::string> endTypes, std::string & _path)
{
	for (unsigned int endTypeIndex = 0; endTypeIndex < endTypes.size(); endTypeIndex++)
	{
		if (endTypes[endTypeIndex] == _path) return true;
		if (endTypes[endTypeIndex].length() > _path.length()) continue;
		int delta = _path.length() - endTypes[endTypeIndex].length();
		bool isSuccess = true;
		for (unsigned int i = 0; i < endTypes[endTypeIndex].length(); ++i)
		{
			std::cout << "\nCompare at : " << i << " , " << endTypes[endTypeIndex][i] << " - " << _path[delta + i];
			if (endTypes[endTypeIndex][i] != _path[delta + i])
			{
				isSuccess = false;
				break;
			}
		}
		if (isSuccess)
			return true;
	}
	return false;
}
