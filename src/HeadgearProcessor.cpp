#include <json/json.h>
#include <json/value.h>
#include <vector>
#include <unordered_set>
#include "HeadgearProcessor.h"
#include "Files.h"
#include "Setup.h"
#include "FormUtil.h"
#include "Workaround.h"
#include "ExclusionManager.h"

namespace HeadgearProcessor
{
	std::unordered_set<RE::TESObjectARMA*> excludedAddons;
	std::unordered_set<RE::TESObjectARMA*> modifiedAddonsHeadband;
	std::unordered_set<RE::TESObjectARMA*> modifiedAddonsNewSlot;
	std::unordered_set<RE::TESObjectARMA*> modifiedAddonsMouth;

	void FixUpHairOnlyArmorSlots(const Setup::TypedSetup& setup)
	{
		auto dataHandler = RE::TESDataHandler::GetSingleton();
		if (dataHandler == NULL)
		{
			return;
		}

		uint32_t hairTopMask = 1 << 0;
		uint32_t hairLongMask = 1 << 1;

		uint32_t headbandMask = 1 << 16;

		uint32_t count = 0;

		const auto& armorArray = dataHandler->GetFormArray<RE::TESObjectARMO>();
    	for (auto* armor : armorArray)
		{
			if (armor == NULL)
			{
				continue;
			}

			auto armorRace = armor->GetFormRace();
			if (armorRace == NULL)
			{
				continue;
			}

			if ((armor->formFlags & 4) != 0)
			{
				continue;
			}

			auto armorSlots = armor->bipedModelData.bipedObjectSlots;
			if (armorSlots != (hairTopMask | hairLongMask) && armorSlots != hairTopMask && armorSlots != hairLongMask)
			{
				continue;
			}

			if (armor->HasKeyword(setup.keyword))
			{
				continue;
			}
			
			armorSlots |= headbandMask;
			armor->bipedModelData.bipedObjectSlots = armorSlots;

			count++;
		}

		REX::INFO(std::format("Adjusted {0} hair-only headgears.", count));

		auto addonsCount = modifiedAddonsHeadband.size() + modifiedAddonsNewSlot.size() + modifiedAddonsMouth.size();
		REX::INFO(std::format("Adjusted {0} headgear addons.",  addonsCount));
	}

	void ScanHeadgearAddons(const Setup::TypedSetup& setup)
	{
		excludedAddons.clear();

		auto dataHandler = RE::TESDataHandler::GetSingleton();
		if (dataHandler == NULL)
		{
			return;
		}

		uint32_t headgearMask = (1 << 0) | (1 << 16) | (1 << 1) | (1 << 28) | (1 << 18) | (1 << 19);
		uint32_t newMask = 1 << (setup.bipedIndex - 30);

		const auto& armorArray = dataHandler->GetFormArray<RE::TESObjectARMO>();
    	for (auto* armor : armorArray)
		{
			if (armor == NULL)
			{
				continue;
			}

			if ((armor->bipedModelData.bipedObjectSlots & headgearMask) == 0)
			{
				continue;
			}

			auto armorRace = armor->GetFormRace();
			if (armorRace == NULL)
			{
				continue;
			}

			uint8_t headband = 0;
			uint8_t newSlot = 0;
			uint8_t mouth = 0;

			uint32_t armorSlots = 0;
			for (const auto& arma : armor->modelArray)
			{
				auto addon = arma.armorAddon;
				if (addon == NULL)
				{
					continue;
				}

				if (addon->GetFormRace() != armorRace)
				{
					bool skip = true;
					for (auto race : addon->additionalRaces)
					{
						if (race == armorRace)
						{
							skip = false;
							break;
						}
					}

					if (skip)
					{
						continue;
					}
				}

				auto slots = addon->bipedModelData.bipedObjectSlots;
				if ((armorSlots & slots) != 0)
				{
					continue;
				}

				armorSlots |= slots;

				if (slots & (1 << 16))
				{
					headband++;
				}
				else if (slots & newMask)
				{
					newSlot++;
				}
				else if (slots & (1 << 19))
				{
					mouth++;
				}
				else if ((slots & (1 << 0)) && (slots & (1 << 1)))
				{
					newSlot++;
				}
				else if (slots & (1 << 0))
				{
					headband++;
				}
				else if (slots & (1 << 1))
				{
					newSlot++;
				}
				else if (slots & (1 << 18))
				{
					mouth++;
				}
			}

			if (headband <= 1 && newSlot <= 1 && mouth <= 1)
			{
				continue;
			}

			REX::WARN(std::format("Headgear [0x{0:08X}] '{1}' contains incompatible addons.", armor->GetFormID(), armor->GetFullName()));
			// REX::WARN(std::format("Headband: {0}, Slot: {1}, Mouth: {2}.", headband, newSlot, mouth));
			for (const auto& arma : armor->modelArray)
			{
				auto addon = arma.armorAddon;
				if (addon == NULL)
				{
					continue;
				}

				if (!excludedAddons.contains(addon))
				{
					excludedAddons.emplace(addon);
				}
			}
		}
	}

	void AddArmorAddon(RE::TESObjectARMO* armor, const Setup::TypedSetup& setup)
	{
		RE::TESObjectARMO::ArmorAddon addonEntry{};

		addonEntry.index = 303;
		addonEntry.armorAddon = setup.armorAddon;

		std::vector<RE::TESObjectARMO::ArmorAddon> addonList;

		addonList.push_back(addonEntry);

		for (auto& ae : armor->modelArray) {
			if (ae.index == 303) {
				REX::WARN(std::format("Form [0x{:08X}] already has armor addon with index 303.", armor->GetFormID()));

				return;
			}

			addonList.push_back(ae);
		}

		armor->modelArray.clear();

		for (auto& ae : addonList) {
			armor->modelArray.push_back(ae);
		}
	}

	void TrySetBaseIndex(RE::TESObjectARMO* armor)
	{
		if (armor == NULL) {
			return;
		}

		if (armor->armorData.index != 0) {
			return;
		}

		bool updateIndex = false;

		for (auto& model : armor->modelArray) {
			if (model.index == 0) {
				updateIndex = true;
				model.index = 1;
			}
		}

		if (updateIndex) {
			armor->armorData.index = 1;
		}
	}

	std::vector<RE::BGSKeyword*> SetArmorBipedIndexes(RE::TESObjectARMO* armor, const Setup::TypedSetup& setup)
	{
		std::vector<RE::BGSKeyword*> res;

		uint32_t hairTopMask = 1 << 0;
		uint32_t hairLongMask = 1 << 1;
		uint32_t hairBeardMask = 1 << 18;

		uint32_t headbandMask = 1 << 16;
		uint32_t newMask = 1 << (setup.bipedIndex - 30);
		uint32_t mouthMask = 1 << 19;

		auto bipedSlots = armor->bipedModelData.bipedObjectSlots;

		if (bipedSlots & hairTopMask)
		{
			res.push_back(setup.keywordHairTop);
		}

		if (bipedSlots & hairLongMask)
		{
			res.push_back(setup.keywordHairLong);
		}

		if (bipedSlots & hairBeardMask)
		{
			res.push_back(setup.keywordHairBeard);
		}

		bipedSlots = bipedSlots & ~hairTopMask;
		bipedSlots = bipedSlots & ~hairLongMask;
		bipedSlots = bipedSlots & ~hairBeardMask;

		for (const auto& arma : armor->modelArray)
		{
			auto addon = arma.armorAddon;
			if (addon == NULL)
			{
				continue;
			}

			if (modifiedAddonsHeadband.contains(addon))
			{
				bipedSlots |= headbandMask;
			}

			if (modifiedAddonsNewSlot.contains(addon))
			{
				bipedSlots |= newMask;
			}

			if (modifiedAddonsMouth.contains(addon))
			{
				bipedSlots |= mouthMask;
			}
		}

		armor->bipedModelData.bipedObjectSlots = bipedSlots;

		return res;
	}

	void ProcessArmorAddons(RE::TESObjectARMO* armor, const Setup::TypedSetup& setup)
	{
		if (armor == NULL)
		{
			return;
		}

		auto armorRace = armor->GetFormRace();
		if (armorRace == NULL)
		{
			return;
		}

		uint32_t headbandMask = 1 << 16;
		uint32_t newMask = 1 << (setup.bipedIndex - 30);
		uint32_t mouthMask = 1 << 19;
		uint32_t hairTopMask = 1 << 0;
		uint32_t hairLongMask = 1 << 1;
		uint32_t beardMask = 1 << 18;

		uint32_t armorSlots = 0;
		for (const auto& arma : armor->modelArray)
		{
			auto addon = arma.armorAddon;
			if (addon == NULL)
			{
				continue;
			}

			if (addon->GetFormRace() != armorRace)
			{
				bool skip = true;
				for (auto race : addon->additionalRaces)
				{
					if (race == armorRace)
					{
						skip = false;
						break;
					}
				}

				if (skip)
				{
					continue;
				}
			}

			auto slots = addon->bipedModelData.bipedObjectSlots;
			if ((armorSlots & slots) != 0)
			{
				// REX::INFO("Skip addon with reused slots.");
				continue;
			}

			armorSlots |= slots;

			if ((slots & (hairTopMask | hairLongMask | beardMask)) == 0)
			{
				// REX::INFO("Skip addon without hair.");
				continue;
			}

			if (slots & headbandMask)
			{
				// REX::INFO("Headband current.");
				if (!modifiedAddonsHeadband.contains(addon))
				{
					modifiedAddonsHeadband.emplace(addon);
				}
			}
			else if (slots & newMask)
			{
				// REX::INFO("Slot current.");
				if (!modifiedAddonsNewSlot.contains(addon))
				{
					modifiedAddonsNewSlot.emplace(addon);
				}
			}
			else if (slots & mouthMask)
			{
				// REX::INFO("Mouth current.");
				if (!modifiedAddonsMouth.contains(addon))
				{
					modifiedAddonsMouth.emplace(addon);
				}
			}
			else if ((slots & hairTopMask) && (slots & hairLongMask))
			{
				// REX::INFO("Combo slot new.");
				slots |= newMask;
				if (!modifiedAddonsNewSlot.contains(addon))
				{
					modifiedAddonsNewSlot.emplace(addon);
				}
			}
			else if (slots & hairTopMask)
			{
				// REX::INFO("Headband new.");
				slots |= headbandMask;
				if (!modifiedAddonsHeadband.contains(addon))
				{
					modifiedAddonsHeadband.emplace(addon);
				}
			}
			else if (slots & hairLongMask)
			{
				// REX::INFO("Slot new.");
				slots |= newMask;
				if (!modifiedAddonsNewSlot.contains(addon))
				{
					modifiedAddonsNewSlot.emplace(addon);
				}
			}
			else if (slots & beardMask)
			{
				// REX::INFO("Mouth new.");
				slots |= mouthMask;
				if (!modifiedAddonsMouth.contains(addon))
				{
					modifiedAddonsMouth.emplace(addon);
				}
			}

			if (slots != addon->bipedModelData.bipedObjectSlots)
			{
				// REX::INFO("Update addon slots.");
				addon->bipedModelData.bipedObjectSlots = slots;
			}
		}
	}

	bool ValidateHeadgear(RE::TESObjectARMO* armor)
	{
		if (armor == NULL)
		{
			return false;
		}

		uint32_t hairTopMask = 1;
		uint32_t hairLongMask = 2;
		uint32_t mask = hairTopMask | hairLongMask;

		auto bipedSlots = armor->bipedModelData.bipedObjectSlots;

		if ((bipedSlots & mask) <= 0)
		{
			return false;
		}

		if (ExclusionManager::Contains(armor))
		{
			REX::WARN(std::format("Headgear [0x{0:08X}] '{1}' is excluded.", armor->GetFormID(), armor->GetFullName()));
			return false;
		}

		auto armorRace = armor->GetFormRace();
		if (armorRace == NULL)
		{
			return false;
		}

		for (const auto& arma : armor->modelArray)
		{
			auto addon = arma.armorAddon;
			if (addon == NULL)
			{
				continue;
			}

			if (excludedAddons.contains(addon))
			{
				REX::ERROR(std::format("Headgear [0x{0:08X}] '{1}' contains excluded addon.", armor->GetFormID(), armor->GetFullName()));
				return false;
			}
		}

		return true;
	}

	void ProcessHeadgearForm(RE::TESForm* form, const Setup::TypedSetup& setup)
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
		if (!ValidateHeadgear(armor))
		{
			return;
		}

		// REX::INFO(std::format("Processing headgear [0x{0:08X}] '{1}'", armor->GetFormID(), armor->GetFullName()));

		ProcessArmorAddons(armor, setup);

		auto keywordsToAdd = SetArmorBipedIndexes(armor, setup);
		for (auto& keyword : keywordsToAdd)
		{
			armor->AddKeyword(keyword);
		}

		armor->AddKeyword(setup.keyword);

		armor->attachParents.AddKeyword(setup.attachSlot);

		TrySetBaseIndex(armor);

		AddArmorAddon(armor, setup);
	}

	void ProcessHeadgearEntry(std::filesystem::directory_entry entry)
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
		if (type != "headgear")
		{
			return;
		}

		auto setup = Setup::GetForms(type);
		if (setup.isEmpty)
		{
			REX::ERROR(std::format("Missing setup for type: {0}.", type));
			return;
		}

		auto keyword = setup.keyword;
		auto attachSlot = setup.attachSlot;
		auto addon = setup.armorAddon;
		auto hairTopKywd = setup.keywordHairTop;
		auto hairLongKywd = setup.keywordHairLong;
		auto hairBearKywd = setup.keywordHairBeard;

		auto formIds = FormUtil::GetFormIdsFromJson(modJson["armorList"]);
		auto forms = FormUtil::GetFormsFromList(formIds);

		if (keyword == NULL || attachSlot == NULL || addon == NULL || hairTopKywd == NULL || hairLongKywd == NULL || hairBearKywd == NULL || forms.empty())
		{
			REX::ERROR(std::format("Incomplete entry for {0}.", path.string()));
			return;
		}

		int count = 0;

		auto filename = path.filename().string();
		REX::INFO(std::format("Processing '{0}'.", filename));

		for (auto& form : forms)
		{
			ProcessHeadgearForm(form, setup);

			count++;
		}

		REX::INFO(std::format("Processed {0} headgear records.", count));
	}

	void ProcessHeadgearFiles()
	{
		auto setup = Setup::GetForms("headgear");
		if (setup.isEmpty || !setup.isEnabled)
		{
			REX::WARN("Headgear support is missing or turned off.");
			return;
		}

		ScanHeadgearAddons(setup);

		auto headgearFiles = Files::GetPluginFiles("Armor");
		for (auto& entry : headgearFiles)
		{
			ProcessHeadgearEntry(entry);
		}

		FixUpHairOnlyArmorSlots(setup);

		excludedAddons.clear();
		modifiedAddonsHeadband.clear();
		modifiedAddonsNewSlot.clear();
		modifiedAddonsMouth.clear();
	}
}
