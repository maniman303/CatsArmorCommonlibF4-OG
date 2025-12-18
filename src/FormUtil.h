#pragma once

#include <json/json.h>
#include <unordered_set>
#include "pch.h"

namespace FormUtil
{
	template <class T>
	T* GetFormAs(uint32_t id)
	{
		auto form = RE::TESForm::GetFormByID(id);
		if (form == NULL)
		{
			return NULL;
		}

		if (!form->Is<T>())
		{
			return NULL;
		}

		return form->As<T>();
	}

	uint32_t GetFormId(std::string modname, uint32_t formid);

	uint32_t GetFormIdFromJson(Json::Value container);

	std::vector<uint32_t> GetFormIdsFromJson(Json::Value container, bool& isAnyMissing);

	std::vector<uint32_t> GetFormIdsFromJson(Json::Value container);

	uint32_t GetItemCount(RE::TESObjectREFR* container, RE::TESForm* itemBase);

	std::string GetHexFormId(int id);

	std::unordered_set<uint32_t> MapToSet(std::vector<uint32_t> vec);

	std::vector<RE::TESForm*> GetFormsFromList(std::vector<uint32_t> formIds);
}
