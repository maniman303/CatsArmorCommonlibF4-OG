#pragma once

#include <json/json.h>
#include <json/value.h>
#include "pch.h"

namespace Setup
{
	class TypedSetup
	{
	public:
		RE::BGSKeyword* keyword;
		RE::BGSKeyword* keywordHidden;
		RE::BGSKeyword* attachSlot;
		RE::TESObjectARMA* armorAddon;
		bool isEmpty;

		TypedSetup(RE::BGSKeyword* kw, RE::BGSKeyword* kwH, RE::BGSKeyword* as, RE::TESObjectARMA* aa);

		TypedSetup(RE::BGSKeyword* kw, RE::BGSKeyword* as, RE::TESObjectARMA* aa);

		TypedSetup();
	};

	bool IsInitialized();

	uint16_t GetAttachmentParentKeywordIndex(RE::BGSKeyword* keyword);

	TypedSetup GetForms(std::string type);

	RE::BGSKeyword* GetActorKeyword();

	bool LoadTypedSetup(Json::Value setup, std::string type);

	void LoadWorkaround(Json::Value setup);

	bool Initialize();
}
