#include <json/json.h>
#include "ArmorProcessor.h"
#include "Workaround.h"
#include "FormUtil.h"
#include "Setup.h"
#include "Files.h"

namespace ArmorProcessor
{
	static void TryModBaseAddonIndex(RE::TESObjectARMO* armor)
	{
		if (armor == NULL)
		{
			return;
		}

		if (armor->armorData.index != 0)
		{
			return;
		}

		bool shouldUpdate = false;

		for (auto& model : armor->modelArray) 
		{
			if (model.index == 0)
			{
				shouldUpdate = true;
				model.index = 1;
			}
		}

		if (shouldUpdate)
		{
			armor->armorData.index = 1;
		}
	}

	static void ProcessArmorForm(RE::TESForm* form, RE::BGSKeyword* keyword, RE::BGSKeyword* attachSlot, RE::TESObjectARMA* addon)
	{
		if (form == NULL)
		{
			return;
		}

		if (form->GetFormType() != RE::ENUM_FORM_ID::kARMO)
		{
			return;
		}

		auto armor = form->As<RE::TESObjectARMO>();

		//REX::INFO(std::format("Adding keyword {0}.", keyword->GetFormID()));

		armor->AddKeyword(keyword);

		//REX::INFO(std::format("Adding attach keyword {0}.", attachSlot->GetFormID()));

		//Workaround::AddAttachKeyword(armor, attachSlot);
		armor->attachParents.AddKeyword(attachSlot);

		//REX::INFO(std::format("Setting up armor addon {0}.", addon->GetFormID()));

		RE::TESObjectARMO::ArmorAddon addonEntry{};

		addonEntry.index = 303;
		addonEntry.armorAddon = addon;

		std::vector<RE::TESObjectARMO::ArmorAddon> addonList;

		addonList.push_back(addonEntry);

		for (auto& ae : armor->modelArray)
		{
			if (ae.index == 303) {
				REX::INFO(std::format("Form {0} already has armor addon with index 303.", FormUtil::GetHexFormId(armor->GetFormID())));
				
				return;
			}

			addonList.push_back(ae);
		}

		armor->modelArray.clear();

		for (auto& ae : addonList)
		{
			armor->modelArray.push_back(ae);
		}

		TryModBaseAddonIndex(armor);
	}

	static void ProcessArmorEntry(std::filesystem::directory_entry entry)
	{
		auto& path = entry.path();

		Json::Value modJson;
		std::ifstream modFile;
		
		try
		{
			modFile.open(path);
			modFile >> modJson;
			modFile.close();
		}
		catch (...)
		{
			REX::ERROR(std::format("Invalid json '{0}'.", path.string()));
			return;
		}

		if (modJson["type"].empty() || !modJson["type"].isString())
		{
			REX::ERROR(std::format("File {0} is missing type.", path.string()));
			return;
		}

		auto type = modJson["type"].asString();
		if (type == "headgear")
		{
			return;
		}

		auto setupForms = Setup::GetForms(type);

		if (setupForms.isEmpty)
		{
			REX::ERROR(std::format("Missing setup for type: {0}.", type));
			return;
		}

		auto keyword = setupForms.keyword;
		auto attachSlot = setupForms.attachSlot;
		auto addon = setupForms.armorAddon;

		auto formIds = FormUtil::GetFormIdsFromJson(modJson["armorList"]);
		auto forms = FormUtil::GetFormsFromList(formIds);

		if (keyword == NULL || attachSlot == NULL || addon == NULL || forms.empty())
		{
			REX::ERROR(std::format("Incomplete entry for {0}.", path.string()));
			return;
		}

		auto filename = path.filename().string();
		REX::INFO(std::format("Processing '{0}'.", filename));

		for (auto& form : forms)
		{
			ProcessArmorForm(form, keyword, attachSlot, addon);
		}
	}

	void ProcessArmorFiles()
	{
		auto pluginFiles = Files::GetPluginFiles("Armor");

		REX::INFO(std::format("Retrieved {0} armor files to process.", pluginFiles.size()));

		for (auto& file : pluginFiles)
		{
			ProcessArmorEntry(file);
		}
	};
}
