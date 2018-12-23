#include "FileExplorer.h"
#include "imgui.h"
#include <vector>
#include <filesystem>
#include <iostream>
#include <string>
FileExplorer FileExplorer::instance;
FileExplorer::FileExplorer()
{
	std::string rootPath = "C:\\";
	for (unsigned int i = 0; i < 5; i++)
	{
		if (std::experimental::filesystem::exists(rootPath))
			roots.push_back(rootPath);
		rootPath[0] = (char)((int)rootPath[0] + 1);
	}
	for (int i = 0; i < roots.size(); i++)
		std::cout << "\nPATH : " << roots[i];
}

void FileExplorer::display()
{
	if (!shouldDisplay)
		return;
	if (isDirty)
	{
		paths.clear();
		for (auto & p : std::experimental::filesystem::directory_iterator(path))
			paths.push_back(p.path().generic_string());
		isDirty = false;
	}
	ImGui::OpenPopup("File Explorer");
	ImGuiWindowFlags window_flags = 0;
	ImGui::SetNextWindowSizeConstraints(ImVec2(470, 420), ImVec2(800, 600));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
	if (ImGui::BeginPopupModal("File Explorer", NULL, window_flags))
	{
		if (ImGui::Button("CLOSE"))
		{
			shouldDisplay = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		for (unsigned int i = 0; i < roots.size(); i++)
		{
			ImGui::SameLine();
			if (ImGui::Button(roots[i].c_str())) { path = roots[i]; isDirty = true; }
		}
		static ImGuiTextFilter filter;
		filter.Draw();
		if (!std::experimental::filesystem::is_empty(path))
		{
			ImGui::BeginChild("directory_files", ImVec2(ImGui::GetWindowContentRegionWidth(), ImGui::GetFrameHeight() - 50), true, ImGuiWindowFlags_HorizontalScrollbar);
			for (std::string strPath : paths)
			{
				std::vector<std::string> filterEnd;
				if (fileFilter != FileType::NONE)
				{
					switch (fileFilter)
					{
					case FileType::IMAGE:
						filterEnd.push_back(".png"); filterEnd.push_back(".jpg");
						break;
					case FileType::TEXT:
						filterEnd.push_back(".txt");
						break;
					default:
						break;
					}
				}

				if (filter.PassFilter(strPath.c_str()))
				{
					bool canShow = true;
					for (unsigned int i = 0; i < filterEnd.size(); i++)
					{
						if (!strPath.find(filterEnd.at(i)))
						{
							canShow = false;
							break;
						}
					}
					if (canShow)
					{
						if (ImGui::Button(strPath.c_str(), ImVec2(ImGui::GetContentRegionAvailWidth(), 30)))
						{
							if (!pathTypeCheck(filterEnd, strPath))
								isDirty = true;
							path = strPath;
						}
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
				std::cout << "\nCalled func to call";
				functionToCall(path);
			}
			shouldDisplay = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::PopStyleVar();
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
	this->fileFilter = fileFilter;
	functionToCall = func;
}

FileExplorer::~FileExplorer()
{
}

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
