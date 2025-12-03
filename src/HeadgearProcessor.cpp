#include <json/json.h>
#include <json/value.h>
#include <vector>
#include "HeadgearProcessor.h"
#include "Files.h"
#include "Setup.h"
#include "FormUtil.h"
#include "Workaround.h"

namespace HeadgearProcessor
{
	bool AdjustHairOnlyHeadgear(RE::TESObjectARMO *armor, Setup::TypedSetup setup)
	{
		uint32_t hairTopMask = 1;
		uint32_t hairLongMask = 2;

		uint32_t mask = hairTopMask;
		mask = mask | hairLongMask;

		uint32_t newSlot = 1 << (setup.bipedIndex - 30);

		if ((armor->formFlags & 4) != 0)
		{
			return false;
		}

		auto bipedSlots = armor->bipedModelData.bipedObjectSlots;

		if ((bipedSlots & ~mask) != 0 || bipedSlots == 0)
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

	void SetArmorAddonBipedIndexes(RE::TESObjectARMA* addon, uint32_t mainBipedSlots, Setup::TypedSetup setup)
	{
		auto bipedSlots = addon->bipedModelData.bipedObjectSlots;

		if ((bipedSlots & mainBipedSlots) == 0) {
			uint32_t newSlot = 1 << (setup.bipedIndex - 30);

			bipedSlots = bipedSlots | newSlot;
		}

		addon->bipedModelData.bipedObjectSlots = bipedSlots;
	}

	std::vector<RE::BGSKeyword*> SetArmorBipedIndexes(RE::TESObjectARMO* armor, Setup::TypedSetup setup)
	{
		std::vector<RE::BGSKeyword*> res;

		uint32_t hairTopMask = 1;
		uint32_t hairLongMask = 2;
		uint32_t hairBeardMask = 1 << 18;
		uint32_t headbandMask = 1 << 16;

		uint32_t newSlot = 1 << (setup.bipedIndex - 30);

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

		bipedSlots = bipedSlots | newSlot;
		bipedSlots = bipedSlots | headbandMask;

		armor->bipedModelData.bipedObjectSlots = bipedSlots;

		auto modelsSize = armor->modelArray.size();
		if (modelsSize > 1)
		{
			// REX::WARN("Many ARMO: {} [{}]", modelsSize, armor->GetFullName());
		}

		for (auto& ae : armor->modelArray) {
			SetArmorAddonBipedIndexes(ae.armorAddon, bipedSlots, setup);
		}

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

		auto forms = FormUtil::GetFormsFromFormListJson(modJson["armorList"], RE::ENUM_FORM_ID::kARMO);

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
