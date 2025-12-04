#include "ActorManager.h"
#include "Setup.h"

bool ActorManager::WornHasKeyword(RE::Actor* actor, RE::BGSKeyword* keyword)
{
    if (actor == NULL || keyword == NULL)
    {
        return false;
    }

    for (auto itemData : actor->inventoryList->data)
    {
        auto object = itemData.object;
        if (object == NULL)
        {
            continue;
        }

        if (!object->Is<RE::TESObjectARMO>())
        {
            continue;
        }

        auto armor = object->As<RE::TESObjectARMO>();

        for (uint32_t i = 0; i < itemData.GetCount(); i++)
        {
            auto stack = itemData.GetStackByID(i);
            if (stack == NULL)
            {
                // REX::INFO("Continued.");
                continue;
            }

            // REX::INFO("Has stack.");

            if (!stack->IsEquipped())
            {
                // REX::INFO("Continued.");
                continue;
            }

            // REX::INFO("Is equipped.");

            if (armor->HasKeyword(keyword))
            {
                return true;
            }

            auto instance = itemData.GetInstanceData(i);
            if (instance == NULL)
            {
                // REX::INFO("Continued.");
                continue;
            }

            // REX::INFO("Has instance data.");

            auto keywordData = instance->GetKeywordData();
            if (keywordData == NULL)
            {
                // REX::INFO("Continued.");
                continue;
            }

            // REX::INFO("Has keyword data.");

            auto keywordIndex = keywordData->GetKeywordIndex(keyword);
            if (keywordIndex.has_value())
            {
                return true;
            }

            // REX::INFO("Didn't have keyword index.");
        }

        // REX::INFO("Next item.");
    }

    return false;
}

bool ActorManager::IsItemEquipped(RE::Actor* actor, RE::BGSObjectInstance instance)
{
    if (actor == NULL || instance.object == NULL)
    {
        return false;
    }

    for (auto itemData : actor->inventoryList->data)
    {
        auto object = itemData.object;
        if (object == NULL || object != instance.object)
        {
            continue;
        }

        for (uint32_t i = 0; i < itemData.GetCount(); i++)
        {
            auto stack = itemData.GetStackByID(i);
            if (stack == NULL)
            {
                continue;
            }

            if (!stack->IsEquipped())
            {
                continue;
            }

            if (!instance.instanceData)
            {
                return true;
            }

            auto expectedInstanceData = instance.instanceData.get();
            if (itemData.GetInstanceData(i) == expectedInstanceData)
            {
                return true;
            }
        }
    }

    return false;
}

bool ActorManager::ProcessHairStubs(RE::Actor* actor, RE::BGSObjectInstance armor, bool isUnequipEvent)
{
    auto setup = Setup::GetForms("headgear");

    bool isVisibleHelmetWorn = ActorManager::WornHasKeyword(actor, setup.keyword) &&
        !ActorManager::WornHasKeyword(actor, setup.keywordHidden);

    bool isEquipped = ActorManager::IsItemEquipped(actor, armor);

    if (!isUnequipEvent && !isEquipped)
    {
        // Skip broken events
        return false;
    }

    // REX::INFO(std::format("Analyze is visible: {0}, is unequip: {1}, is equipped: {2}, form id: {3}", isVisibleHelmetWorn, isUnequipEvent, isEquipped, armor.object->GetFormID()));

    auto instanceHairTop = RE::BGSObjectInstance(setup.armorHairTop, NULL);
    auto instanceHairLong = RE::BGSObjectInstance(setup.armorHairLong, NULL);
    auto instanceHairBeard = RE::BGSObjectInstance(setup.armorHairBeard, NULL);

    auto equipManager = RE::ActorEquipManager::GetSingleton();

    if (!isVisibleHelmetWorn || !isEquipped)
    {
        equipManager->UnequipObject(actor, &instanceHairTop, 1, NULL, 0, true, true, false, true, NULL);
        equipManager->UnequipObject(actor, &instanceHairLong, 1, NULL, 0, true, true, false, true, NULL);
        equipManager->UnequipObject(actor, &instanceHairBeard, 1, NULL, 0, true, true, false, true, NULL);

        actor->Reset3D(true, 0, true, 0xC);

        return isUnequipEvent != isEquipped;
    }

    bool res = true;

    if (ActorManager::WornHasKeyword(actor, setup.keywordHairTop))
    {
        res = res && equipManager->EquipObject(actor, instanceHairTop, 0, 1, NULL, true, true, false, true, true);
    }

    if (ActorManager::WornHasKeyword(actor, setup.keywordHairLong))
    {
        res = res && equipManager->EquipObject(actor, instanceHairLong, 0, 1, NULL, true, true, false, true, true);
    }

    if (ActorManager::WornHasKeyword(actor, setup.keywordHairBeard))
    {
        res = res && equipManager->EquipObject(actor, instanceHairBeard, 0, 1, NULL, true, true, false, true, true);
    }

    actor->Reset3D(true, 0, true, 0xC);

    return res && (isUnequipEvent != isEquipped);
}