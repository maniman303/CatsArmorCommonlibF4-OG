#pragma once

#include "PCH.h"

namespace ActorManager
{
    enum BoolResult
    {
        kFalse = 0,
        kTrue = 1,
        kError = 2
    };

    bool WornHasKeyword(RE::Actor* actor, RE::BGSKeyword* keyword, const RE::TBO_InstanceData* instance = NULL);

    BoolResult SafeWornHasKeyword(RE::Actor* actor, RE::BGSKeyword* keyword, const RE::TBO_InstanceData* instance = NULL);

    bool IsItemEquipped(RE::Actor* actor, const RE::BGSObjectInstance* instance);

    bool UnequipItem(RE::Actor* actor, RE::TESObjectARMO* armor);

    bool EquipItem(RE::Actor* actor, RE::TESObjectARMO* armor);
}