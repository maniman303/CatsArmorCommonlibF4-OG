#pragma once

#include "PCH.h"

namespace ActorManager
{
    bool WornHasKeyword(RE::Actor* actor, RE::BGSKeyword* keyword);

    bool IsItemEquipped(RE::Actor* actor, RE::BGSObjectInstance instance);
}