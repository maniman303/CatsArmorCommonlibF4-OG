#pragma once

#include "pch.h"

namespace Hooks
{
	struct InitLoadGame
	{
		static void thunk(RE::Actor* aThis, RE::BGSLoadFormBuffer* buf);

		static inline REL::Relocation<decltype(thunk)> Hook;
	};

	struct ShouldBackgroundClone
	{
		static bool thunk(RE::Actor* aThis);

		static inline REL::Relocation<decltype(thunk)> Hook;
	};

	struct Revert
	{
		static void thunk(RE::Actor* aThis, RE::BGSLoadFormBuffer* buf);

		static inline REL::Relocation<decltype(thunk)> Hook;
	};

	struct LoadGame
	{
		static void thunk(RE::Actor* aThis, RE::BGSLoadFormBuffer* buf);

		static inline REL::Relocation<decltype(thunk)> Hook;
	};

	void Install();
}
