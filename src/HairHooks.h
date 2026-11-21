#pragma once

#include "PCH.h"

namespace HairHooks
{
	// FUN_1405AE640 (RVA 0x5AE640) is the engine function that applies the
	// hide/show decision to a hair, hair long or beard head part: it walks the
	// head part geometry and toggles the subsegments tagged with 0x1E + slot.
	// Its only caller is the equip/head update function that contains the three
	// calls below (around RVA 0x3FF000); it sees every actor including the
	// player, in both directions.
	//
	// The thunk resolves the owning actor from the node's userData (walking the
	// parent chain) and replaces the engine's hide flag with the value returned
	// by ShouldHide(actor, slot, engineHide). Change ShouldHide to control what
	// is rendered; it receives the actor, so the decision can read the equipped
	// item for the slot from a_actor->biped->object[slot].parent.object.
	//
	// FUN_1401C6DE0 is a second, independent hide path: the biped pass
	// (FUN_1401C3530) calls it (through FUN_1401C3900) to hide the hair/beard
	// slots that share the worn headgear's ARMA. HeadPartHide intercepts it so
	// ShouldHide stays the single source of truth.
	//
	// FUN_1405AE730 is a third path for a head part's extraParts (hairline,
	// etc.); it takes a { face node, slot, hide } context by pointer, so
	// HeadPartExtra rewrites the hide flag in place before the original runs.
	//
	// No editor IDs or node names are used anywhere.
	struct HeadPartVisibility
	{
		using func_t = void(RE::NiAVObject*, std::int32_t, bool);

		static void thunk(RE::NiAVObject* a_node, std::int32_t a_slot, bool a_hide);

		// The three FUN_1405AE640 calls inside the equip/head update caller:
		// hair, hair long, beard. Each value is the RVA of the CALL instruction
		// to redirect (OG 1.10.163).
		static constexpr std::size_t kHookHairOffset{ 0x3FF058 };
		static constexpr std::size_t kHookHairLongOffset{ 0x3FF0CE };
		static constexpr std::size_t kHookBeardOffset{ 0x3FF19A };

		// Resolved in Install() to the original function each call targeted.
		static inline REL::Relocation<func_t> HookHair{};
		static inline REL::Relocation<func_t> HookHairLong{};
		static inline REL::Relocation<func_t> HookBeard{};
	};

	struct HeadPartHide
	{
		using func_t = void(RE::NiAVObject*, std::int32_t**);

		static void thunk(RE::NiAVObject* a_face, std::int32_t** a_slot);

		// FUN_1401C6DE0 call inside FUN_1401C3900 (itself called by the biped
		// pass FUN_1401C3530). The value is the RVA of the CALL instruction to
		// redirect (OG 1.10.163).
		static constexpr std::size_t kHookOffset{ 0x1C3983 };

		// Resolved in Install() to the original function the call targeted.
		static inline REL::Relocation<func_t> Hook{};
	};

	// Context FUN_1405AE730 takes by pointer: the face node, the biped slot and
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

		// The three FUN_1405AE730 calls inside the equip/head update caller: the
		// extra parts (hairline, ...) of hair, hair long and beard. Each value is
		// the RVA of the CALL instruction to redirect (OG 1.10.163).
		static constexpr std::size_t kHookHairOffset{ 0x3FF098 };
		static constexpr std::size_t kHookHairLongOffset{ 0x3FF10B };
		static constexpr std::size_t kHookBeardOffset{ 0x3FF1D8 };

		// Resolved in Install() to the original function each call targeted.
		static inline REL::Relocation<func_t> HookHair{};
		static inline REL::Relocation<func_t> HookHairLong{};
		static inline REL::Relocation<func_t> HookBeard{};
	};

	void Install();
}
