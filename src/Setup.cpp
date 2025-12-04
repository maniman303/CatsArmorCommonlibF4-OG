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

		auto keyword = FormUtil::GetFormFromJson(typedSetup["keywordToAdd"], RE::ENUM_FORM_ID::kKYWD);
		if (keyword == NULL || keyword->GetFormType() != RE::ENUM_FORM_ID::kKYWD) {
			return false;
		}

		auto attachSlot = FormUtil::GetFormFromJson(typedSetup["attachSlotToAdd"], RE::ENUM_FORM_ID::kKYWD);
		if (attachSlot == NULL || attachSlot->GetFormType() != RE::ENUM_FORM_ID::kKYWD) {
			return false;
		}

		auto armorAddon = FormUtil::GetFormFromJson(typedSetup["armorAddon"], RE::ENUM_FORM_ID::kARMA);
		if (armorAddon == NULL || armorAddon->GetFormType() != RE::ENUM_FORM_ID::kARMA) {
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

		auto keyword = FormUtil::GetFormFromJson(typedSetup["keywordToAdd"], RE::ENUM_FORM_ID::kKYWD);
		if (keyword == NULL || keyword->GetFormType() != RE::ENUM_FORM_ID::kKYWD) {
			return false;
		}

		auto keywordHidden = FormUtil::GetFormFromJson(typedSetup["keywordHidden"], RE::ENUM_FORM_ID::kKYWD);
		if (keywordHidden == NULL || keywordHidden->GetFormType() != RE::ENUM_FORM_ID::kKYWD) {
			return false;
		}

		auto attachSlot = FormUtil::GetFormFromJson(typedSetup["attachSlotToAdd"], RE::ENUM_FORM_ID::kKYWD);
		if (attachSlot == NULL || attachSlot->GetFormType() != RE::ENUM_FORM_ID::kKYWD) {
			return false;
		}

		auto armorAddon = FormUtil::GetFormFromJson(typedSetup["armorAddon"], RE::ENUM_FORM_ID::kARMA);
		if (armorAddon == NULL || armorAddon->GetFormType() != RE::ENUM_FORM_ID::kARMA) {
			return false;
		}

		auto keywordHairLong = FormUtil::GetFormFromJson(typedSetup["keywordHairLong"], RE::ENUM_FORM_ID::kKYWD);
		if (keywordHairLong == NULL || keywordHairLong->GetFormType() != RE::ENUM_FORM_ID::kKYWD) {
			return false;
		}

		auto keywordHairTop = FormUtil::GetFormFromJson(typedSetup["keywordHairTop"], RE::ENUM_FORM_ID::kKYWD);
		if (keywordHairTop == NULL || keywordHairTop->GetFormType() != RE::ENUM_FORM_ID::kKYWD) {
			return false;
		}

		auto keywordHairBeard = FormUtil::GetFormFromJson(typedSetup["keywordHairBeard"], RE::ENUM_FORM_ID::kKYWD);
		if (keywordHairBeard == NULL || keywordHairBeard->GetFormType() != RE::ENUM_FORM_ID::kKYWD) {
			return false;
		}

		auto armorHairLong = FormUtil::GetFormFromJson(typedSetup["armorHairLong"], RE::ENUM_FORM_ID::kARMO);
		if (armorHairLong == NULL || armorHairLong->GetFormType() != RE::ENUM_FORM_ID::kARMO) {
			return false;
		}

		auto armorHairTop = FormUtil::GetFormFromJson(typedSetup["armorHairTop"], RE::ENUM_FORM_ID::kARMO);
		if (armorHairTop == NULL || armorHairTop->GetFormType() != RE::ENUM_FORM_ID::kARMO) {
			return false;
		}

		auto armorHairBeard = FormUtil::GetFormFromJson(typedSetup["armorHairBeard"], RE::ENUM_FORM_ID::kARMO);
		if (armorHairBeard == NULL || armorHairBeard->GetFormType() != RE::ENUM_FORM_ID::kARMO) {
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

		auto spell = FormUtil::GetFormFromJson(magicSetup["spell"], RE::ENUM_FORM_ID::kSPEL);
		if (spell == NULL || spell->GetFormType() != RE::ENUM_FORM_ID::kSPEL) {
			REX::WARN("Magic setup is missing spell.");
			return false;
		}

		auto perk = FormUtil::GetFormFromJson(magicSetup["perk"], RE::ENUM_FORM_ID::kPERK);
		if (perk == NULL || perk->GetFormType() != RE::ENUM_FORM_ID::kPERK) {
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

		auto form = FormUtil::GetFormFromJson(actorSetup, RE::ENUM_FORM_ID::kKYWD);

		if (form == NULL) {
			return false;
		}

		auto kywd = form->As<RE::BGSKeyword>();

		ActorKywd = kywd;

		return true;
	}

	void LoadWorkaround(Json::Value setup)
	{
		auto workaroundSetup = setup["workaround"];

		if (workaroundSetup.empty()) {
			return;
		}

		auto armor = FormUtil::GetFormFromJson(workaroundSetup, RE::ENUM_FORM_ID::kARMO);

		if (armor == NULL) {
			REX::WARN("Workaround setup has invalid item.");
			return;
		}

		auto form = armor->As<RE::TESObjectARMO>();

		if (form->attachParents.size != 6) {
			REX::WARN("Workaround armor has invalid amount of attach parent slots.");
			return;
		}

		auto torsoKeyword = SetupMap["torso"].attachSlot->GetFormID();
		auto leftArmKeyword = SetupMap["leftArm"].attachSlot->GetFormID();
		auto rightArmKeyword = SetupMap["rightArm"].attachSlot->GetFormID();
		auto leftLegKeyword = SetupMap["leftLeg"].attachSlot->GetFormID();
		auto rightLegKeyword = SetupMap["rightLeg"].attachSlot->GetFormID();

		WorkaroundMap[torsoKeyword] = form->attachParents.array[0].keywordIndex;
		WorkaroundMap[leftArmKeyword] = form->attachParents.array[1].keywordIndex;
		WorkaroundMap[rightArmKeyword] = form->attachParents.array[2].keywordIndex;
		WorkaroundMap[leftLegKeyword] = form->attachParents.array[3].keywordIndex;
		WorkaroundMap[rightLegKeyword] = form->attachParents.array[4].keywordIndex;

		if (SetupMap.contains("headgear") && SetupMap["headgear"].attachSlot != NULL) {
			WorkaroundMap[SetupMap["headgear"].attachSlot->GetFormID()] = form->attachParents.array[5].keywordIndex;
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
