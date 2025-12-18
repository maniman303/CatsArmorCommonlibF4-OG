#include "FormUtil.h"

namespace FormUtil
{
	uint32_t GetFormId(std::string modname, uint32_t formid)
	{
		if (!modname.length() || !formid) {
			return 0;
		}

		uint32_t id = formid;

		RE::TESDataHandler* dh = RE::TESDataHandler::GetSingleton();
		auto modIndexOpt = dh->GetLoadedModIndex(modname);

		if (!modIndexOpt.has_value()) {
			auto lightModIndexOpt = dh->GetLoadedLightModIndex(modname);

			if (!lightModIndexOpt.has_value()) {
				return 0;
			}

			uint16_t lightModIndex = lightModIndexOpt.value();
			if (lightModIndex != 0xFFFF) {
				id |= 0xFE000000 | (uint32_t(lightModIndex) << 12);
			}
		} else {
			auto modIndex = modIndexOpt.value();
			id |= ((uint32_t)modIndex) << 24;
		}

		return id;
	}

	uint32_t GetFormIdFromString(std::string key)
	{
		if (key.empty()) {
			REX::INFO("Key is empty.");
			return 0;
		}

		auto splits = StringHelper::SplitString(key, '|');

		if (splits.size() != 2) {
			REX::INFO(std::format("Invalid form key {0}.", key));
			return 0;
		}

		auto modName = splits.front();
		splits.pop_front();

		auto formIdHex = splits.front();
		int formId = std::stoi(formIdHex, 0, 16);

		auto form = GetFormId(modName, formId);

		return form;
	}

	uint32_t GetFormIdFromJson(Json::Value container)
	{
		if (container.empty()) {
			return NULL;
		}

		if (container.isString()) {
			return GetFormIdFromString(container.asString());
		}

		auto keys = container.getMemberNames();

		for (auto& key : keys) {
			auto formIdValue = container[key];

			if (!formIdValue.isInt()) {
				continue;
			}

			auto formId = formIdValue.asInt();
			auto form = GetFormId(key, formId);

			if (form != 0) {
				return form;
			}
		}

		return 0;
	}

	std::vector<uint32_t> GetFormIdsFromJson(Json::Value container, bool& isAnyMissing)
	{
		isAnyMissing = false;

		std::vector<uint32_t> result;

		if (container.empty())
		{
			return result;
		}

		if (container.isString())
		{
			auto stringForm = GetFormIdFromString(container.asString());

			if (stringForm == 0) {
				isAnyMissing = true;
				REX::WARN(std::format("Could not find [{0}] form id.", container.asString()));
				return result;
			}

			result.push_back(stringForm);
			return result;
		}

		if (container.isArray())
		{
			for (auto value: container)
			{
				if (!value.isString())
				{
					continue;
				}

				auto stringForm = GetFormIdFromString(value.asString());

				if (stringForm == 0) {
					isAnyMissing = true;
					REX::WARN(std::format("Could not find [{0}] form id.", value.asString()));
					continue;
				}

				result.push_back(stringForm);
			}

			return result;
		}

		auto keys = container.getMemberNames();

		for (auto& key : keys)
		{
			auto formIdValue = container[key];

			if (!formIdValue.isInt())
			{
				continue;
			}

			auto formId = formIdValue.asInt();
			auto form = GetFormId(key, formId);

			if (form != 0)
			{
				result.push_back(form);
			}
		}

		return result;
	}

	std::vector<uint32_t> GetFormIdsFromJson(Json::Value container)
	{
		bool isAnyMissing;

		return GetFormIdsFromJson(container, isAnyMissing);
	}

	uint32_t GetItemCount(RE::TESObjectREFR* container, RE::TESForm* itemBase)
	{
		uint32_t count = 0;

		auto formId = itemBase->GetFormID();
		auto items = container->inventoryList->data;
		int inventorySize = items.size();
		for (int i = 0; i < inventorySize; i++) {
			auto item = items.at(i);
			if (item.object->GetFormID() == formId) {
				count += item.GetCount();
			}
		}

		return count;
	}

	std::string GetHexFormId(int id)
	{
		std::stringstream ss;
		ss << std::hex << id;

		return ss.str();
	}

	std::unordered_set<uint32_t> MapToSet(std::vector<uint32_t> vec)
	{
		std::unordered_set<uint32_t> set;
		set.reserve(vec.size());

		for (uint32_t v : vec)
		{
			set.insert(v);
		}

		return set;
	}

	std::vector<RE::TESForm*> GetFormsFromList(std::vector<uint32_t> formIds)
	{
		std::vector<RE::TESForm*> res;

		for (auto formId : formIds)
		{
			auto form = RE::TESForm::GetFormByID(formId);
			if (form == NULL)
			{
				continue;
			}

			if (!form->Is(RE::ENUM_FORM_ID::kFLST))
			{
				res.push_back(form);

				continue;
			}

			auto formList = form->As<RE::BGSListForm>();

			for (auto& formFromList : formList->arrayOfForms)
			{
				if (formFromList != NULL)
				{
					res.push_back(formFromList);
				}
			}
		}

		return res;
	}
}
