#pragma once

#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include "pch.h"

namespace Files
{
	bool IsFilePresent();

	std::filesystem::path GetRootPath();

	std::filesystem::path GetPluginPath();

	bool VerifyPaths();

	bool VerifyCatsPlugin();

	std::vector<std::filesystem::directory_entry> GetPluginFiles(std::string path);

	void PrepareDirectories();
}
