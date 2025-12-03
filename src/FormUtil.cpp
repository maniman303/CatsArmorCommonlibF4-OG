#include "FormUtil.h"

namespace FormUtil
{
	RE::TESForm* GetFormFromMod(std::string modname, uint32_t formid)
	{
		if (!modname.length() || !formid) {
			return NULL;
		}

		uint32_t id = formid;

		RE::TESDataHandler* dh = RE::TESDataHandler::GetSingleton();
		auto modIndexOpt = dh->GetLoadedModIndex(modname);

		if (!modIndexOpt.has_value()) {
			auto lightModIndexOpt = dh->GetLoadedLightModIndex(modname);

			if (!lightModIndexOpt.has_value()) {
				REX::INFO(std::format("Mod '{0}' is not loaded.", modname));
				return NULL;
			}

			uint16_t lightModIndex = lightModIndexOpt.value();
			if (lightModIndex != 0xFFFF) {
				id |= 0xFE000000 | (uint32_t(lightModIndex) << 12);
			}
		} else {
			auto modIndex = modIndexOpt.value();
			id |= ((uint32_t)modIndex) << 24;
		}

		return RE::TESForm::GetFormByID(id);
	}

	RE::TESForm* GetFormFromMod(std::string modname, uint32_t formid, RE::ENUM_FORM_ID type)
	{
		auto form = GetFormFromMod(modname, formid);

		if (form == NULL) {
			return NULL;
		}

		if (form->GetFormType() != type) {
			REX::INFO("Form has wrong type.");
			return NULL;
		}

		return form;
	}

	RE::TESForm* GetFormFromString(std::string key, RE::ENUM_FORM_ID type)
	{
		if (key.empty()) {
			REX::INFO("Key is empty.");
			return NULL;
		}

		auto splits = StringHelper::SplitString(key, '|');

		if (splits.size() != 2) {
			REX::INFO(std::format("Invalid form key {0}.", key));
			return NULL;
		}

		auto modName = splits.front();
		splits.pop_front();

		auto formIdHex = splits.front();
		int formId = std::stoi(formIdHex, 0, 16);

		auto form = GetFormFromMod(modName, formId);

		if (form == NULL) {
			REX::INFO(std::format("Form from mod {0} with id {1} doesn't exists.", modName, formId));
			return NULL;
		}

		if (form->GetFormType() != type) {
			REX::INFO("Form has wrong type.");
			return NULL;
		}

		return form;
	}

	RE::TESForm* GetFormFromJson(Json::Value container, RE::ENUM_FORM_ID type)
	{
		if (container.empty()) {
			REX::INFO("Empty json.");
			return NULL;
		}

		if (container.isString()) {
			return GetFormFromString(container.asString(), type);
		}

		auto keys = container.getMemberNames();

		for (auto& key : keys) {
			auto formIdValue = container[key];

			if (!formIdValue.isInt()) {
				continue;
			}

			auto formId = formIdValue.asInt();
			auto form = GetFormFromMod(key, formId, type);

			if (form != NULL) {
				return form;
			}
		}

		REX::INFO("Could not find matching form.");

		return NULL;
	}

	std::vector<RE::TESForm*> GetFormsFromJson(Json::Value container, RE::ENUM_FORM_ID type)
	{
		std::vector<RE::TESForm*> result;

		if (container.empty())
		{
			return result;
		}

		if (container.isString())
		{
			auto stringForm = GetFormFromString(container.asString(), type);

			if (stringForm == NULL) {
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

				auto stringForm = GetFormFromString(value.asString(), type);

				if (stringForm == NULL) {
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
			auto form = GetFormFromMod(key, formId, type);

			if (form != NULL)
			{
				result.push_back(form);
			}
		}

		return result;
	}

	std::vector<RE::TESForm*> GetFormsFromFormListJson(Json::Value container, RE::ENUM_FORM_ID type)
	{
		std::vector<RE::TESForm*> result;
		auto formLists = GetFormsFromJson(container, RE::ENUM_FORM_ID::kFLST);

		for (auto& formList: formLists)
		{
			if (formList == NULL || formList->GetFormType() != RE::ENUM_FORM_ID::kFLST)
			{
				continue;
			}

			auto list = formList->As<RE::BGSListForm>();

			for (auto& form : list->arrayOfForms)
			{
				if (form == NULL)
				{
					continue;
				}

				if (form->GetFormType() != type)
				{
					continue;
				}

				result.push_back(form);
			}
		}

		return result;
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
}
