#include "Workaround.h"
#include "Setup.h"

namespace Workaround
{
	void AddKeyword(RE::BGSKeywordForm* form, RE::BGSKeyword* keyword)
	{
		if (form == NULL || keyword == NULL) {
			return;
		}

		auto numKeywords = form->GetNumKeywords();
		auto keywords = form->keywords;

		for (uint32_t i = 0; i < numKeywords; i++) {
			if (keywords[i]->GetFormID() == keyword->GetFormID()) {
				return;
			}
		}

		auto newKeywords = (RE::BGSKeyword**)malloc(sizeof(RE::BGSKeyword*) * (numKeywords + 1));

		if (newKeywords == NULL) {
			return;
		}

		for (uint32_t i = 0; i < numKeywords; i++) {
			newKeywords[i] = keywords[i];
		}

		newKeywords[numKeywords] = keyword;

		form->keywords = newKeywords;
		form->numKeywords += 1;

		RE::free(keywords);
	}

	bool HasKeyword(RE::BGSAttachParentArray* attachParent, RE::BGSKeyword* keyword)
	{
		auto newKwdIndex = Setup::GetAttachmentParentKeywordIndex(keyword);

		for (uint32_t i = 0; i < attachParent->size; i++) {
			auto kwdIndex = attachParent->array[i].keywordIndex;
			if (kwdIndex == newKwdIndex) {
				return true;
			}
		}

		return false;
	}

	void AddAttachKeyword(RE::TESObjectARMO* form, RE::BGSKeyword* keyword)
	{
		if (form == NULL || keyword == NULL) {
			REX::WARN("Invalid parameters.");
			return;
		}

		auto newKeywordIndex = Setup::GetAttachmentParentKeywordIndex(keyword);
		if (newKeywordIndex == 0) {
			REX::WARN("Keyword index not found in workaround.");
			return;
		}

		if (HasKeyword(&form->attachParents, keyword)) {
			REX::INFO("Form already has attach parent keyword.");
			return;
		}

		auto newArray = (RE::BGSTypedKeywordValue<RE::KeywordType::kAttachPoint>*)malloc(sizeof(RE::BGSTypedKeywordValue<RE::KeywordType::kAttachPoint>) * (form->attachParents.size + 1));
		for (uint32_t i = 0; i < form->attachParents.size; i++) {
			newArray[i] = form->attachParents.array[i];
		}

		RE::BGSTypedKeywordValue<RE::KeywordType::kAttachPoint> newValue{};
		newValue.keywordIndex = newKeywordIndex;
		newArray[form->attachParents.size] = newValue;

		form->attachParents.array = newArray;
		form->attachParents.size = form->attachParents.size + 1;

		RE::free(form->attachParents.array);

		//REX::INFO(std::format("Added attach parent keyword with index {0}.", newValue.keywordIndex));
	}
}
