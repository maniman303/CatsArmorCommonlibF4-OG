#pragma once

#include <json/json.h>
#include <json/value.h>
#include "pch.h"

namespace Setup
{
	static int DefaultBipedIndex = 58;

	class TypedSetup
	{
	public:
		RE::BGSKeyword* keywordHairLong;
		RE::BGSKeyword* keywordHairTop;
		RE::BGSKeyword* keywordHairBeard;
		RE::BGSKeyword* keyword;
		RE::BGSKeyword* attachSlot;
		RE::TESObjectARMA* armorAddon;
		int bipedIndex;
		bool isEmpty;
		bool isEnabled;

		TypedSetup(int bi, RE::BGSKeyword* kw, RE::BGSKeyword* as, RE::TESObjectARMA* aa, RE::BGSKeyword* kwHl, RE::BGSKeyword* kwHt, RE::BGSKeyword* kwHb);

		TypedSetup(RE::BGSKeyword* kw, RE::BGSKeyword* as, RE::TESObjectARMA* aa);

		TypedSetup();
	};

	bool IsInitialized();

	uint16_t GetAttachmentParentKeywordIndex(RE::BGSKeyword* keyword);

	TypedSetup GetForms(std::string type);

	RE::SpellItem* GetSpell();

	RE::BGSPerk* GetPerk();

	RE::BGSKeyword* GetActorKeyword();

	bool LoadTypedSetup(Json::Value setup, std::string type);

	void LoadWorkaround(Json::Value setup);

	bool Initialize();
}
