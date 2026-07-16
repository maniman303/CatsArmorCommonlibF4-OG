#pragma once

#include "PCH.h"

namespace ActorManager
{
    bool WornHasKeyword(RE::Actor* actor, RE::BGSKeyword* keyword, const RE::TBO_InstanceData* instance = NULL);

    bool IsItemEquipped(RE::Actor* actor, const RE::BGSObjectInstance* instance);

    bool UnequipItem(RE::Actor* actor, RE::TESObjectARMO* armor);

    bool EquipItem(RE::Actor* actor, RE::TESObjectARMO* armor);

    bool ProcessHairStubs(RE::Actor* actor, const RE::BGSObjectInstance* armor, bool isUnequipEvent);
}