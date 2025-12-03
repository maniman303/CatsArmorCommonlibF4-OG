#pragma once

#include <json/json.h>
#include "pch.h"

namespace FormUtil
{
	RE::TESForm* GetFormFromMod(std::string modname, uint32_t formid);

	RE::TESForm* GetFormFromMod(std::string modname, uint32_t formid, RE::ENUM_FORM_ID type);

	RE::TESForm* GetFormFromString(std::string key, RE::ENUM_FORM_ID type);

	RE::TESForm* GetFormFromJson(Json::Value container, RE::ENUM_FORM_ID type);

	std::vector<RE::TESForm*> GetFormsFromJson(Json::Value container, RE::ENUM_FORM_ID type);

	std::vector<RE::TESForm*> GetFormsFromFormListJson(Json::Value container, RE::ENUM_FORM_ID type);

	uint32_t GetItemCount(RE::TESObjectREFR* container, RE::TESForm* itemBase);

	std::string GetHexFormId(int id);
}
