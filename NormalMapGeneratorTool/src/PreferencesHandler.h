#pragma once
#include <string>
#include <fstream>

struct PreferenceInfo
{
	int maxWidthRes, maxHeightRes;
	int maxUndoCount;
	std::string defaultExportPath;
	std::string defaultTheme;
};

class PreferencesHandler
{
private:
	static std::string m_filePath;
public:

	static void init(const std::string& filePath)
	{
		m_filePath = filePath;
	}

	static void savePreferences(int maxResWidth, int maxResHeight, int maxUndoSteps, const std::string& defaultPath, const std::string& defaultTheme)
	{
		std::ofstream file;
		file.open(m_filePath, std::ios::out | std::ios::trunc);

		if (file.is_open())
		{
			file << maxResWidth << "," << maxResHeight << "," << maxUndoSteps << "," << defaultPath << "," << defaultTheme;
			file.close();
		}
	}

	static void savePreferences(const PreferenceInfo& info)
	{
		savePreferences(info.maxWidthRes, info.maxHeightRes, info.maxUndoCount, info.defaultExportPath, info.defaultTheme);
	}

	static void setDefaults()
	{
		savePreferences(4096, 4096, 20, "C:\\NoraOutput.tga", "Default");
	}

	static PreferenceInfo readPreferences()
	{
		std::ifstream file;
		file.open(m_filePath);

		std::string finalStr = "";
		if (file.is_open())
		{
			std::string line;
			while (getline(file, line))
			{
				finalStr += line;
			}
			file.close();
		}

		int indexOfFirstComma = finalStr.find(',');
		int indexOfSecondComma = finalStr.find(',', indexOfFirstComma + 1);
		int indexOfThirdComma = finalStr.find(',', indexOfSecondComma + 1);
		int indexOfFourthComma = finalStr.find(',', indexOfThirdComma + 1);
		std::string str1 = finalStr.substr(0, indexOfFirstComma);
		std::string str2 = finalStr.substr(indexOfFirstComma + 1, indexOfSecondComma - (indexOfFirstComma + 1));
		std::string str3 = finalStr.substr(indexOfSecondComma + 1, indexOfThirdComma - (indexOfSecondComma + 1));
		std::string str4 = finalStr.substr(indexOfThirdComma + 1, indexOfFourthComma - (indexOfThirdComma + 1));
		std::string str5 = finalStr.substr(indexOfFourthComma + 1);

		PreferenceInfo info;
		info.maxWidthRes = std::stoi(str1);
		info.maxHeightRes = std::stoi(str2);
		info.maxUndoCount = std::stoi(str3);
		info.defaultExportPath = str4;
		info.defaultTheme = str5;

		return info;
	}
};
std::string PreferencesHandler::m_filePath = "";