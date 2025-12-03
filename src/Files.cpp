#include "Files.h"

namespace Files
{
	bool isFilePresent = false;

	bool IsFilePresent()
	{
		return isFilePresent;
	}

	std::filesystem::path GetRootPath()
	{
		auto workingDir = std::filesystem::current_path();
		workingDir.append("Data");
		workingDir.append("F4SE");

		return workingDir;
	}

	std::filesystem::path GetPluginPath()
	{
		auto rootDir = Files::GetRootPath();
		rootDir.append("Plugins");
		rootDir.append("CatsArmor");

		return rootDir;
	}

	bool VerifyPaths()
	{
		auto pluginPath = Files::GetRootPath();
		bool pathExists = std::filesystem::exists(pluginPath);

		return pathExists;
	}

	bool VerifyCatsPlugin()
	{
		isFilePresent = false;

		RE::TESDataHandler* dh = RE::TESDataHandler::GetSingleton();
		auto modIndexOpt = dh->GetLoadedLightModIndex("Cats Armor.esl");

		if (!modIndexOpt.has_value()) {
			return false;
		}

		isFilePresent = true;

		return true;
	}

	std::vector<std::filesystem::directory_entry> GetPluginFiles(std::string path)
	{
		std::vector<std::filesystem::directory_entry> result;

		auto pluginPath = Files::GetPluginPath().append(path);

		if (!std::filesystem::exists(pluginPath) || pluginPath.empty()) {
			REX::WARN("Specified folder does not exist.");
			return result;
		}

		for (auto& entry : std::filesystem::directory_iterator(pluginPath)) {
			if (entry.is_directory()) {
				continue;
			}

			auto pathToFile = entry.path();

			if (StringHelper::ToLower(pathToFile.extension().string()) != ".json") {
				continue;
			}

			if (StringHelper::StartsWith(StringHelper::ToLower(pathToFile.filename().string()), "_")) {
				continue;
			}

			result.push_back(entry);
		}

		return result;
	}

	void PrepareDirectories()
	{
		auto pluginPath = Files::GetPluginPath();
		bool pathExists = std::filesystem::exists(pluginPath);

		if (!pathExists) {
			std::filesystem::create_directory(pluginPath);
		}

		auto setupPath = Files::GetPluginPath().append("Setup");

		if (!std::filesystem::exists(setupPath)) {
			std::filesystem::create_directory(setupPath);
		}

		auto armorPath = Files::GetPluginPath().append("Armor");

		if (!std::filesystem::exists(armorPath)) {
			std::filesystem::create_directory(armorPath);
		}
	};
}
