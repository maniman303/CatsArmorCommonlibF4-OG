#include <json/json.h>
#include <json/value.h>
#include <vector>
#include "HeadgearProcessor.h"
#include "Files.h"
#include "Setup.h"
#include "FormUtil.h"
#include "Workaround.h"
#include "ExclusionManager.h"

namespace HeadgearProcessor
{
	bool AdjustHairOnlyHeadgear(RE::TESObjectARMO *armor, Setup::TypedSetup setup)
	{
		if (armor == NULL)
		{
			return false;
		}

		uint32_t hairTopMask = 1;
		uint32_t hairLongMask = 2;
		uint32_t mask = hairTopMask | hairLongMask;

		uint32_t newSlot = 1 << 16;

		if ((armor->formFlags & 4) != 0)
		{
			return false;
		}

		auto bipedSlots = armor->bipedModelData.bipedObjectSlots;
		if ((bipedSlots & ~mask) > 0 || bipedSlots == 0)
		{
			return false;
		}

		if (armor->HasKeyword(setup.keyword))
		{
			return false;
		}

		bipedSlots = bipedSlots | newSlot;
		armor->bipedModelData.bipedObjectSlots = bipedSlots;

		return true;
	}

	void ProcessHairOnlyHeadgear(Setup::TypedSetup setup)
	{
		auto dataHandler = RE::TESDataHandler::GetSingleton();

		if (dataHandler == NULL)
		{
			return;
		}

		const auto& armorArray = dataHandler->GetFormArray<RE::TESObjectARMO>();

		int adjusted = 0;

    	for (auto* armor : armorArray)
		{
			if (AdjustHairOnlyHeadgear(armor, setup))
			{
				adjusted++;

				// REX::INFO("Wrong headgear [{}]", armor->GetFullName());
			}
		}

		REX::INFO(std::format("Adjusted {} headgears.", adjusted));
	}

	void SetArmorAddonBipedIndexes(RE::TESObjectARMO* armor, uint32_t newSlot)
	{
		if (armor == NULL)
		{
			return;
		}

		uint32_t hairTopMask = 1;
		uint32_t hairLongMask = 2;
		uint32_t hairBeardMask = 1 << 18;
		uint32_t mask = hairTopMask | hairLongMask | hairBeardMask;

		uint32_t slotsTaken = 0;

		for (auto& arma : armor->modelArray)
		{
			auto addon = arma.armorAddon;
			if (addon == NULL)
			{
				continue;
			}

			auto addonSlots = addon->bipedModelData.bipedObjectSlots;
			if ((addonSlots & slotsTaken) > 0)
			{
				continue;
			}

			slotsTaken = slotsTaken | addonSlots;

			if ((addonSlots & ~mask) > 0)
			{
				continue;
			}

			addonSlots = addonSlots | newSlot;
			addon->bipedModelData.bipedObjectSlots = addonSlots;
		}
	}

	std::vector<RE::BGSKeyword*> SetArmorBipedIndexes(RE::TESObjectARMO* armor, Setup::TypedSetup setup)
	{
		std::vector<RE::BGSKeyword*> res;

		uint32_t hairTopMask = 1;
		uint32_t hairLongMask = 2;
		uint32_t hairBeardMask = 1 << 18;
		uint32_t headbandMask = 1 << 16;

		uint32_t setupSlot = 1 << (setup.bipedIndex - 30);

		// REX::INFO("Processing [{}].", armor->GetFullName());

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

		uint32_t newAddonSlot = headbandMask;

		for (auto& ae : armor->modelArray)
		{
			if (ae.armorAddon == NULL)
			{
				continue;
			}

			auto addonSlots = ae.armorAddon->bipedModelData.bipedObjectSlots;
			if ((addonSlots & headbandMask) > 0)
			{
				newAddonSlot = setupSlot;
			}
		}

		bipedSlots = bipedSlots | newAddonSlot;

		armor->bipedModelData.bipedObjectSlots = bipedSlots;

		SetArmorAddonBipedIndexes(armor, newAddonSlot);

		return res;
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

	void AddArmorAddon(RE::TESObjectARMO* armor, Setup::TypedSetup setup)
	{
		RE::TESObjectARMO::ArmorAddon addonEntry{};

		addonEntry.index = 303;
		addonEntry.armorAddon = setup.armorAddon;

		std::vector<RE::TESObjectARMO::ArmorAddon> addonList;

		addonList.push_back(addonEntry);

		for (auto& ae : armor->modelArray) {
			if (ae.index == 303) {
				REX::INFO(std::format("Form {0} already has armor addon with index 303.", FormUtil::GetHexFormId(armor->GetFormID())));

				return;
			}

			addonList.push_back(ae);
		}

		armor->modelArray.clear();

		for (auto& ae : addonList) {
			armor->modelArray.push_back(ae);
		}
	}

	bool ValidateArmorAddons(RE::TESObjectARMO* armor, Setup::TypedSetup setup)
	{
		auto armorSlots = armor->bipedModelData.bipedObjectSlots;

		uint32_t hairTopMask = 1;
		uint32_t hairLongMask = 2;
		uint32_t hairBeardMask = 1 << 18;
		uint32_t mask = hairTopMask | hairLongMask | hairBeardMask;

		uint32_t headbandMask = 1 << 16;
		uint32_t extraMask = 1 << (setup.bipedIndex - 30);

		int addonsWithOnlyHair = 0;
		int addonsWithHeadband = 0;
		int addonsWithExtra = 0;

		uint32_t slotsTaken = 0;

		for (auto& arma : armor->modelArray)
		{
			auto addon = arma.armorAddon;
			if (addon == NULL)
			{
				continue;
			}

			auto addonSlots = addon->bipedModelData.bipedObjectSlots;
			if ((addonSlots & armorSlots) == 0 || (addonSlots & slotsTaken) > 0)
			{
				continue;
			}

			slotsTaken = slotsTaken | addonSlots;

			if ((addonSlots & ~mask) == 0)
			{
				addonsWithOnlyHair++;
			}

			if ((addonSlots & headbandMask) > 0)
			{
				addonsWithHeadband++;
			}

			if ((addonSlots & extraMask) > 0)
			{
				addonsWithExtra++;
			}
		}

		if (addonsWithOnlyHair > 1)
		{
			REX::ERROR(std::format("Headgear [{0}] has too many hair only armor addons.", armor->GetFullName()));
			return false;
		}

		if (addonsWithOnlyHair > 0 && addonsWithHeadband > 0 && addonsWithExtra > 0)
		{
			REX::ERROR(std::format("Headgear [{0}] has incompatible armor addon setup.", armor->GetFullName()));
			return false;
		}

		return true;
	}

	bool ValidateHeadgear(RE::TESObjectARMO* armor, Setup::TypedSetup setup)
	{
		if (ExclusionManager::Contains(armor))
		{
			REX::INFO(std::format("Excluded [{0}].", armor->GetFullName()));
			return false;
		}

		uint32_t hairTopMask = 1;
		uint32_t hairLongMask = 2;
		uint32_t mask = hairTopMask | hairLongMask;

		auto bipedSlots = armor->bipedModelData.bipedObjectSlots;

		if ((bipedSlots & mask) <= 0)
		{
			REX::WARN(std::format("Headgear [{0}] with id 0x{1:X} does not take hair slots.", armor->GetFullName(), armor->GetFormID()));
			return false;
		}

		return ValidateArmorAddons(armor, setup);
	}

	void ProcessHeadgearForm(RE::TESForm* form, Setup::TypedSetup setup)
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

		if (!ValidateHeadgear(armor, setup))
		{
			return;
		}

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

		auto setupForms = Setup::GetForms(type);

		if (setupForms.isEmpty)
		{
			REX::ERROR(std::format("Missing setup for type: {0}.", type));
			return;
		}

		auto keyword = setupForms.keyword;
		auto attachSlot = setupForms.attachSlot;
		auto addon = setupForms.armorAddon;
		auto hairTopKywd = setupForms.keywordHairTop;
		auto hairLongKywd = setupForms.keywordHairLong;
		auto hairBearKywd = setupForms.keywordHairBeard;

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
			ProcessHeadgearForm(form, setupForms);

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

		auto headgearFiles = Files::GetPluginFiles("Armor");

		for (auto& entry : headgearFiles)
		{
			ProcessHeadgearEntry(entry);
		}

		ProcessHairOnlyHeadgear(setup);
	}
}
