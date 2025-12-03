#include "Hooks.h"
#include "PerkDistributor.h"

namespace Hooks
{
	void InitLoadGame::thunk(RE::Actor* aThis, RE::BGSLoadFormBuffer* buf)
	{
		InitLoadGame::Hook(aThis, buf);

		if (aThis == NULL)
		{
			return;
		}

		auto npc = aThis->GetNPC();
		if (npc == NULL)
		{
			return;
		}

		auto niObj = aThis->GetFullyLoaded3D();
		if (niObj == NULL)
		{
			return;
		}

		if (PerkDistributor::TryProcessNpc(npc))
		{
			// REX::INFO("Hook: InitLoadGame");
		}
	}

	bool ShouldBackgroundClone::thunk(RE::Actor* aThis)
	{
		if (aThis == NULL)
		{
			return ShouldBackgroundClone::Hook(aThis);
		}

		auto npc = aThis->GetNPC();
		if (npc == NULL)
		{
			return ShouldBackgroundClone::Hook(aThis);
		}

		if (PerkDistributor::TryProcessNpc(npc))
		{
			// REX::INFO("Hook: ShouldBackgroundClone");
		}

		return ShouldBackgroundClone::Hook(aThis);
	}

	void Revert::thunk(RE::Actor* aThis, RE::BGSLoadFormBuffer* buf)
	{
		Revert::Hook(aThis, buf);

		if (aThis == NULL)
		{
			return;
		}

		auto npc = aThis->GetNPC();
		if (npc == NULL)
		{
			return;
		}

		PerkDistributor::TryRevertActor(aThis);
	}

	void LoadGame::thunk(RE::Actor* aThis, RE::BGSLoadFormBuffer* buf)
	{
		if (aThis == NULL)
		{
			return;
		}

		auto npc = aThis->GetNPC();
		if (npc == NULL)
		{
			LoadGame::Hook(aThis, buf);

			return;
		}

		if (PerkDistributor::TryProcessNpc(npc))
		{
			// REX::INFO("Hook: LoadGame");
		}

		LoadGame::Hook(aThis, buf);
	}

	bool _hooked = false;

	void Install()
	{
		if (_hooked)
		{
			return;
		}

		REL::Relocation<std::uintptr_t> target {RE::Actor::VTABLE[0]};

		InitLoadGame::Hook = target.write_vfunc(0x13, reinterpret_cast<std::uintptr_t>(InitLoadGame::thunk));
		ShouldBackgroundClone::Hook = target.write_vfunc(0x89, reinterpret_cast<std::uintptr_t>(ShouldBackgroundClone::thunk));
		Revert::Hook = target.write_vfunc(0x15, reinterpret_cast<std::uintptr_t>(Revert::thunk));
		LoadGame::Hook = target.write_vfunc(0x12, reinterpret_cast<std::uintptr_t>(LoadGame::thunk));

		REX::INFO("Hooks installed.");

		_hooked = true;
	}
}
