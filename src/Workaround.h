#pragma once

#include "pch.h"

namespace Workaround
{
	void AddKeyword(RE::BGSKeywordForm* form, RE::BGSKeyword* keyword);

	bool HasKeyword(RE::BGSAttachParentArray* attachParent, RE::BGSKeyword* keyword);

	void AddAttachKeyword(RE::TESObjectARMO* form, RE::BGSKeyword* keyword);
}
