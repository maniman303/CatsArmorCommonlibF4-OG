#pragma once

#include "PCH.h"

namespace HairHooks
{
	// FUN_14064E480 (RVA 0x64E480) is the engine function that applies the
	// hide/show decision to a hair, hair long or beard head part: it walks the
	// head part geometry and toggles the subsegments tagged with 0x1E + slot.
	// Its only caller is FUN_140506460, which runs from the equip update and the
	// reference/head update, so the three calls below see every actor including
	// the player, in both directions.
	//
	// The thunk resolves the owning actor from the node's userData (walking the
	// parent chain) and replaces the engine's hide flag with the value returned
	// by ShouldHide(actor, slot, engineHide). Change ShouldHide to control what
	// is rendered; it receives the actor, so the decision can read the equipped
	// item for the slot from a_actor->biped->object[slot].parent.object.
	//
	// FUN_140360540 is a second, independent hide path: the biped pass
	// (FUN_14035E3B0) calls it to hide the hair/beard slots that share the worn
	// headgear's ARMA. HeadPartHide intercepts it so ShouldHide stays the single
	// source of truth.
	//
	// FUN_14064E570 is a third path for a head part's extraParts (hairline,
	// etc.); it takes a { face node, slot, hide } context by pointer, so
	// HeadPartExtra rewrites the hide flag in place before the original runs.
	//
	// No editor IDs or node names are used anywhere.
	struct HeadPartVisibility
	{
		using func_t = void(RE::NiAVObject*, std::int32_t, bool);

		static void thunk(RE::NiAVObject* a_node, std::int32_t a_slot, bool a_hide);

		// The three FUN_14064E480 calls inside FUN_140506460: hair, hair long,
		// beard. Each value is the RVA of the CALL instruction to redirect (AE).
		static constexpr std::size_t kHookHairOffset{ 0x506733 };
		static constexpr std::size_t kHookHairLongOffset{ 0x5067A3 };
		static constexpr std::size_t kHookBeardOffset{ 0x506863 };

		// Resolved in Install() to the original function each call targeted.
		static inline REL::Relocation<func_t> HookHair{};
		static inline REL::Relocation<func_t> HookHairLong{};
		static inline REL::Relocation<func_t> HookBeard{};
	};

	struct HeadPartHide
	{
		using func_t = void(RE::NiAVObject*, std::int32_t**);

		static void thunk(RE::NiAVObject* a_face, std::int32_t** a_slot);

		// FUN_140360540 call inside FUN_14035E3B0. The value is the RVA of the
		// CALL instruction to redirect (AE).
		static constexpr std::size_t kHookOffset{ 0x35E796 };

		// Resolved in Install() to the original function the call targeted.
		static inline REL::Relocation<func_t> Hook{};
	};

	// Context FUN_14064E570 takes by pointer: the face node, the biped slot and
	// the hide flag it applies to the extra part's subsegments.
	struct HeadPartContext
	{
		RE::NiAVObject* face;   // 00
		std::int32_t    slot;   // 08
		bool            hide;   // 0C
	};
	static_assert(sizeof(HeadPartContext) == 0x10);
	static_assert(offsetof(HeadPartContext, face) == 0x00);
	static_assert(offsetof(HeadPartContext, slot) == 0x08);
	static_assert(offsetof(HeadPartContext, hide) == 0x0C);

	struct HeadPartExtra
	{
		using func_t = std::uintptr_t(HeadPartContext*, RE::BGSHeadPart*);

		static std::uintptr_t thunk(HeadPartContext* a_ctx, RE::BGSHeadPart* a_extraPart);

		// The three FUN_14064E570 calls inside FUN_140506460: the extra parts
		// (hairline, ...) of hair, hair long and beard. Each value is the RVA of
		// the CALL instruction to redirect (AE).
		static constexpr std::size_t kHookHairOffset{ 0x506768 };
		static constexpr std::size_t kHookHairLongOffset{ 0x5067D8 };
		static constexpr std::size_t kHookBeardOffset{ 0x506898 };

		// Resolved in Install() to the original function each call targeted.
		static inline REL::Relocation<func_t> HookHair{};
		static inline REL::Relocation<func_t> HookHairLong{};
		static inline REL::Relocation<func_t> HookBeard{};
	};

	void Install();
}
