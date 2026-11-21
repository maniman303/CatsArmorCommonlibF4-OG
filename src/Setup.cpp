#include <iostream>
#include <fstream>
#include <map>
#include "Setup.h"
#include "Files.h"
#include "FormUtil.h"

namespace Setup
{
	std::map<int, uint16_t> WorkaroundMap;
	RE::BGSKeyword* ActorKywd = NULL;
	bool IsInitializedFlag = false;

	bool IsInitialized()
	{
		return IsInitializedFlag;
	}

	uint16_t GetAttachmentParentKeywordIndex(RE::BGSKeyword* keyword)
	{
		if (keyword == NULL) {
			return 0;
		}

		int formId = keyword->GetFormID();

		if (WorkaroundMap.contains(formId)) {
			return WorkaroundMap[formId];
		}

		return 0;
	}

	TypedSetup::TypedSetup(RE::BGSKeyword* kw, RE::BGSKeyword* kwH, RE::BGSKeyword* as, RE::TESObjectARMA* aa)
	{
		keyword = kw;
		keywordHidden = kwH;
		attachSlot = as;
		armorAddon = aa;
		isEmpty = false;
	}

	TypedSetup::TypedSetup(RE::BGSKeyword* kw, RE::BGSKeyword* as, RE::TESObjectARMA* aa)
	{
		keyword = kw;
		keywordHidden = NULL;
		attachSlot = as;
		armorAddon = aa;
		isEmpty = false;
	}

	TypedSetup::TypedSetup()
	{
		keyword = NULL;
		keywordHidden = NULL;
		attachSlot = NULL;
		armorAddon = NULL;
		isEmpty = true;
	}

	std::map<std::string, TypedSetup> SetupMap;

	TypedSetup GetForms(std::string type)
	{
		for (auto& it : SetupMap) {
			if (it.first.compare(type) == 0) {
				return it.second;
			}
		}

		return TypedSetup();
	}

	RE::BGSKeyword* GetActorKeyword()
	{
		return ActorKywd;
	}

	bool LoadTypedSetup(Json::Value setup, std::string type)
	{
		auto typedSetup = setup[type];

		if (typedSetup.empty() || !typedSetup.isObject())
		{
			return false;
		}

		auto keywordId = FormUtil::GetFormIdFromJson(typedSetup["keywordToAdd"]);
		auto keyword = FormUtil::GetFormAs<RE::BGSKeyword>(keywordId);
		if (keyword == NULL)
		{
			return false;
		}

		auto keywordHiddenId = FormUtil::GetFormIdFromJson(typedSetup["keywordHidden"]);
		auto keywordHidden = FormUtil::GetFormAs<RE::BGSKeyword>(keywordHiddenId);

		auto attachSlotId = FormUtil::GetFormIdFromJson(typedSetup["attachSlotToAdd"]);
		auto attachSlot = FormUtil::GetFormAs<RE::BGSKeyword>(attachSlotId);
		if (attachSlot == NULL)
		{
			return false;
		}

		auto armorAddonId = FormUtil::GetFormIdFromJson(typedSetup["armorAddon"]);
		auto armorAddon = FormUtil::GetFormAs<RE::TESObjectARMA>(armorAddonId);
		if (armorAddon == NULL)
		{
			return false;
		}

		SetupMap[type] = TypedSetup(keyword, keywordHidden, attachSlot, armorAddon);

		return true;
	}

	bool LoadActorKeyword(Json::Value setup)
	{
		ActorKywd = NULL;

		auto actorSetup = setup["actorProcessing"];

		if (actorSetup.empty()) {
			return false;
		}

		auto formId = FormUtil::GetFormIdFromJson(actorSetup);
		auto form = FormUtil::GetFormAs<RE::BGSKeyword>(formId);

		if (form == NULL) {
			return false;
		}

		ActorKywd = form;

		return true;
	}

	void LoadWorkaround(Json::Value setup)
	{
		auto workaroundSetup = setup["workaround"];

		if (workaroundSetup.empty()) {
			return;
		}

		auto armorId = FormUtil::GetFormIdFromJson(workaroundSetup);
		auto armor = FormUtil::GetFormAs<RE::TESObjectARMO>(armorId);

		if (armor == NULL) {
			REX::WARN("Workaround setup has invalid item.");
			return;
		}

		if (armor->attachParents.size != 6) {
			REX::WARN("Workaround armor has invalid amount of attach parent slots.");
			return;
		}

		auto torsoKeyword = SetupMap["torso"].attachSlot->GetFormID();
		auto leftArmKeyword = SetupMap["leftArm"].attachSlot->GetFormID();
		auto rightArmKeyword = SetupMap["rightArm"].attachSlot->GetFormID();
		auto leftLegKeyword = SetupMap["leftLeg"].attachSlot->GetFormID();
		auto rightLegKeyword = SetupMap["rightLeg"].attachSlot->GetFormID();

		WorkaroundMap[torsoKeyword] = armor->attachParents.array[0].keywordIndex;
		WorkaroundMap[leftArmKeyword] = armor->attachParents.array[1].keywordIndex;
		WorkaroundMap[rightArmKeyword] = armor->attachParents.array[2].keywordIndex;
		WorkaroundMap[leftLegKeyword] = armor->attachParents.array[3].keywordIndex;
		WorkaroundMap[rightLegKeyword] = armor->attachParents.array[4].keywordIndex;

		if (SetupMap.contains("headgear") && SetupMap["headgear"].attachSlot != NULL) {
			WorkaroundMap[SetupMap["headgear"].attachSlot->GetFormID()] = armor->attachParents.array[5].keywordIndex;
		}
	}

	bool Initialize()
	{
		IsInitializedFlag = false;

		SetupMap.clear();
		
		auto path = Files::GetPluginPath().append("Setup");
		auto filePath = path.append("Setup.json");

		if (!std::filesystem::exists(filePath)) {
			REX::INFO("Missing Setup.json file.");
			return false;
		}

		Json::Value setupJson;
		std::ifstream setupFile;

		try
		{
			setupFile.open(filePath);

			setupFile >> setupJson;

			setupFile.close();
		}
		catch (std::exception ex)
		{
			REX::ERROR(std::format("Invalid json '{0}'.", filePath.string()));

			return false;
		}

		bool result = true;

		for (const std::string& key : setupJson.getMemberNames())
		{
			if (key == "actorProcessing")
			{
				continue;
			}
			else if (key == "magic")
			{
				continue;
			}

			// REX::INFO(std::format("Trying to load [{0}].", key));

			if (!LoadTypedSetup(setupJson, key))
			{
				REX::WARN(std::format("Invalid setup for [{0}].", key));
				result = false;
			}
		}

		if (!LoadActorKeyword(setupJson))
		{
			REX::WARN("Missing setup for actor processing.");
			result = false;
		}

		IsInitializedFlag = result;

		LoadWorkaround(setupJson);

		return result;
	}
}
