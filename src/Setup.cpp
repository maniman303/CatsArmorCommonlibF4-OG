#include <iostream>
#include <fstream>
#include <map>
#include "Setup.h"
#include "Files.h"
#include "FormUtil.h"

namespace Setup
{
	std::map<int, uint16_t> WorkaroundMap;
	RE::SpellItem* Spell = NULL;
	RE::BGSPerk* Perk = NULL;
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

	TypedSetup::TypedSetup(int bi, RE::BGSKeyword* kw, RE::BGSKeyword* kwH, RE::BGSKeyword* as, RE::TESObjectARMA* aa,
		RE::BGSKeyword* kwHl, RE::BGSKeyword* kwHt, RE::BGSKeyword* kwHb, RE::TESObjectARMO* aHl, RE::TESObjectARMO* aHt, RE::TESObjectARMO* aHb)
	{
		keyword = kw;
		keywordHidden = kwH;
		attachSlot = as;
		armorAddon = aa;
		isEmpty = false;
		isEnabled = true;
		bipedIndex = bi,
		keywordHairLong = kwHl;
		keywordHairTop = kwHt;
		keywordHairBeard = kwHb;
		armorHairLong = aHl;
		armorHairTop = aHt;
		armorHairBeard = aHb;
	}

	TypedSetup::TypedSetup(RE::BGSKeyword* kw, RE::BGSKeyword* as, RE::TESObjectARMA* aa)
	{
		keyword = kw;
		keywordHidden = NULL;
		attachSlot = as;
		armorAddon = aa;
		isEmpty = false;
		isEnabled = true;
		bipedIndex = DefaultBipedIndex;
		keywordHairLong = NULL;
		keywordHairTop = NULL;
		keywordHairBeard = NULL;
		armorHairLong = NULL;
		armorHairTop = NULL;
		armorHairBeard = NULL;
	}

	TypedSetup::TypedSetup()
	{
		keyword = NULL;
		keywordHidden = NULL;
		attachSlot = NULL;
		armorAddon = NULL;
		isEmpty = true;
		isEnabled = false;
		bipedIndex = DefaultBipedIndex;
		keywordHairLong = NULL;
		keywordHairTop = NULL;
		keywordHairBeard = NULL;
		armorHairLong = NULL;
		armorHairTop = NULL;
		armorHairBeard = NULL;
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

	RE::SpellItem* GetSpell()
	{
		return Spell;
	}

	RE::BGSPerk* GetPerk()
	{
		return Perk;
	}

	RE::BGSKeyword* GetActorKeyword()
	{
		return ActorKywd;
	}

	bool LoadTypedSetup(Json::Value setup, std::string type)
	{
		auto typedSetup = setup[type];

		if (typedSetup.empty()) {
			return false;
		}

		auto keywordId = FormUtil::GetFormIdFromJson(typedSetup["keywordToAdd"]);
		auto keyword = FormUtil::GetFormAs<RE::BGSKeyword>(keywordId);
		if (keyword == NULL) {
			return false;
		}

		auto attachSlotId = FormUtil::GetFormIdFromJson(typedSetup["attachSlotToAdd"]);
		auto attachSlot = FormUtil::GetFormAs<RE::BGSKeyword>(attachSlotId);
		if (attachSlot == NULL) {
			return false;
		}

		auto armorAddonId = FormUtil::GetFormIdFromJson(typedSetup["armorAddon"]);
		auto armorAddon = FormUtil::GetFormAs<RE::TESObjectARMA>(armorAddonId);
		if (armorAddon == NULL) {
			return false;
		}

		SetupMap[type] = TypedSetup(keyword->As<RE::BGSKeyword>(), attachSlot->As<RE::BGSKeyword>(), armorAddon->As<RE::TESObjectARMA>());

		return true;
	}

	bool LoadHeadgearSetup(Json::Value setup)
	{
		auto typedSetup = setup["headgear"];

		if (typedSetup.empty()) {
			REX::WARN("Json setup is empty.");
			return false;
		}

		auto enabledSetup = typedSetup["enabled"];
		if (!enabledSetup.empty() && enabledSetup.isBool() && !enabledSetup.asBool()) {
			REX::WARN("Json setup is disabled.");
			return false;
		}

		int bipedIndex = DefaultBipedIndex;
		auto bipedIndexSetup = typedSetup["bipedIndex"];
		if (!bipedIndexSetup.empty() && bipedIndexSetup.isInt()) {
			bipedIndex = bipedIndexSetup.asInt();
			REX::WARN(std::format("Custom biped index set to: {}.", bipedIndex));
		}

		auto keywordId = FormUtil::GetFormIdFromJson(typedSetup["keywordToAdd"]);
		auto keyword = FormUtil::GetFormAs<RE::BGSKeyword>(keywordId);
		if (keyword == NULL) {
			return false;
		}

		auto keywordHiddenId = FormUtil::GetFormIdFromJson(typedSetup["keywordHidden"]);
		auto keywordHidden = FormUtil::GetFormAs<RE::BGSKeyword>(keywordHiddenId);
		if (keywordHidden == NULL) {
			return false;
		}

		auto attachSlotId = FormUtil::GetFormIdFromJson(typedSetup["attachSlotToAdd"]);
		auto attachSlot = FormUtil::GetFormAs<RE::BGSKeyword>(attachSlotId);
		if (attachSlot == NULL) {
			return false;
		}

		auto armorAddonId = FormUtil::GetFormIdFromJson(typedSetup["armorAddon"]);
		auto armorAddon = FormUtil::GetFormAs<RE::TESObjectARMA>(armorAddonId);
		if (armorAddon == NULL) {
			return false;
		}

		auto keywordHairLongId = FormUtil::GetFormIdFromJson(typedSetup["keywordHairLong"]);
		auto keywordHairLong = FormUtil::GetFormAs<RE::BGSKeyword>(keywordHairLongId);
		if (keywordHairLong == NULL) {
			return false;
		}

		auto keywordHairTopId = FormUtil::GetFormIdFromJson(typedSetup["keywordHairTop"]);
		auto keywordHairTop = FormUtil::GetFormAs<RE::BGSKeyword>(keywordHairTopId);
		if (keywordHairTop == NULL) {
			return false;
		}

		auto keywordHairBeardId = FormUtil::GetFormIdFromJson(typedSetup["keywordHairBeard"]);
		auto keywordHairBeard = FormUtil::GetFormAs<RE::BGSKeyword>(keywordHairBeardId);
		if (keywordHairBeard == NULL) {
			return false;
		}

		auto armorHairLongId = FormUtil::GetFormIdFromJson(typedSetup["armorHairLong"]);
		auto armorHairLong = FormUtil::GetFormAs<RE::TESObjectARMO>(armorHairLongId);
		if (armorHairLong == NULL) {
			return false;
		}

		auto armorHairTopId = FormUtil::GetFormIdFromJson(typedSetup["armorHairTop"]);
		auto armorHairTop = FormUtil::GetFormAs<RE::TESObjectARMO>(armorHairTopId);
		if (armorHairTop == NULL) {
			return false;
		}

		auto armorHairBeardId = FormUtil::GetFormIdFromJson(typedSetup["armorHairBeard"]);
		auto armorHairBeard = FormUtil::GetFormAs<RE::TESObjectARMO>(armorHairBeardId);
		if (armorHairBeard == NULL) {
			return false;
		}

		SetupMap["headgear"] = TypedSetup(bipedIndex, keyword->As<RE::BGSKeyword>(), keywordHidden->As<RE::BGSKeyword>(), attachSlot->As<RE::BGSKeyword>(), armorAddon->As<RE::TESObjectARMA>(),
			keywordHairLong->As<RE::BGSKeyword>(), keywordHairTop->As<RE::BGSKeyword>(), keywordHairBeard->As<RE::BGSKeyword>(),
			armorHairLong->As<RE::TESObjectARMO>(), armorHairTop->As<RE::TESObjectARMO>(), armorHairBeard->As<RE::TESObjectARMO>());

		return true;
	}

	bool LoadSpellAndPerk(Json::Value setup)
	{
		Spell = NULL;
		Perk = NULL;

		auto magicSetup = setup["magic"];

		if (magicSetup.empty()) {
			REX::WARN("Magic setup is missing.");
			return false;
		}

		auto spellId = FormUtil::GetFormIdFromJson(magicSetup["spell"]);
		auto spell = FormUtil::GetFormAs<RE::SpellItem>(spellId);
		if (spell == NULL) {
			REX::WARN("Magic setup is missing spell.");
			return false;
		}

		auto perkId = FormUtil::GetFormIdFromJson(magicSetup["perk"]);
		auto perk = FormUtil::GetFormAs<RE::BGSPerk>(perkId);
		if (perk == NULL) {
			REX::WARN("Magic setup is missing perk.");
			return false;
		}

		Spell = spell->As<RE::SpellItem>();
		Perk = perk->As<RE::BGSPerk>();

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

		try {
			setupFile.open(filePath);

			setupFile >> setupJson;

			setupFile.close();
		} catch (std::exception ex) {
			REX::ERROR(std::format("Invalid json '{0}'.", filePath.string()));

			return false;
		}

		bool result = true;

		if (!LoadTypedSetup(setupJson, "torso")) {
			REX::WARN("Missing setup for torso.");
			result = false;
		}

		if (!LoadTypedSetup(setupJson, "leftArm")) {
			REX::WARN("Missing setup for left arm.");
			result = false;
		}

		if (!LoadTypedSetup(setupJson, "rightArm")) {
			REX::WARN("Missing setup for right arm.");
			result = false;
		}

		if (!LoadTypedSetup(setupJson, "leftLeg")) {
			REX::WARN("Missing setup for left leg.");
			result = false;
		}

		if (!LoadTypedSetup(setupJson, "rightLeg")) {
			REX::WARN("Missing setup for right leg.");
			result = false;
		}

		if (!LoadHeadgearSetup(setupJson)) {
			REX::WARN("Missing setup for headgear.");
		}

		if (!LoadSpellAndPerk(setupJson))
		{
			REX::WARN("Missing setup for spell and perk.");
			result = false;
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
