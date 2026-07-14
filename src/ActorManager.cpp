#include "ActorManager.h"
#include "Setup.h"

uint32_t CountStacks(const RE::BGSInventoryItem& itemData)
{
    uint32_t res = 0;

    auto stack = itemData.stackData.get();
    while (stack != NULL)
    {
        res++;
        stack = stack->nextStack.get();
    }

    return res;
}

bool ActorManager::WornHasKeyword(RE::Actor* actor, RE::BGSKeyword* keyword)
{
    if (actor == NULL || keyword == NULL)
    {
        return false;
    }

    auto inventoryList = actor->inventoryList;
    if (inventoryList == NULL)
    {
        auto npc = actor->GetNPC();
        if (npc != NULL)
        {
            REX::WARN(std::format("Inventory for actor [{0}] is NULL.", npc->GetFullName()));
        }
        
        return false;
    }

    for (auto& itemData : inventoryList->data)
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

        for (uint32_t i = 0; i < CountStacks(itemData); i++)
        {
            auto stack = itemData.GetStackByID(i);
            if (stack == NULL)
            {
                // REX::INFO("Stack is null, continued.");
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

bool ActorManager::IsItemEquipped(RE::Actor* actor, const RE::BGSObjectInstance* instance)
{
    if (actor == NULL || instance == NULL || instance->object == NULL)
    {
        return false;
    }

    for (auto itemData : actor->inventoryList->data)
    {
        auto object = itemData.object;
        if (object == NULL || object != instance->object)
        {
            continue;
        }

        for (uint32_t i = 0; i < CountStacks(itemData); i++)
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

            if (!instance->instanceData)
            {
                return true;
            }

            auto expectedInstanceData = instance->instanceData.get();
            if (itemData.GetInstanceData(i) == expectedInstanceData)
            {
                return true;
            }
        }
    }

    return false;
}

bool ActorManager::ProcessHairStubs(RE::Actor* actor, const RE::BGSObjectInstance* armor, bool isUnequipEvent)
{
    auto setup = Setup::GetForms("headgear");
    if (setup.isEmpty)
    {
        return true;
    }

    bool isVisibleHelmetWorn = ActorManager::WornHasKeyword(actor, setup.keyword) &&
        !ActorManager::WornHasKeyword(actor, setup.keywordHidden);

    bool isEquipped = ActorManager::IsItemEquipped(actor, armor);

    if (!isUnequipEvent && !isEquipped)
    {
        // Skip broken events
        return false;
    }

    // REX::INFO(std::format("Analyze is visible: {0}, is unequip: {1}, is equipped: {2}, form id: {3}", isVisibleHelmetWorn, isUnequipEvent, isEquipped, armor.object->GetFormID()));

    auto armorHairTop = setup.armorHairTop;
    auto armorHairLong = setup.armorHairLong;
    auto armorHairBeard = setup.armorHairBeard;
    auto instanceHairTop = new RE::BGSObjectInstance(armorHairTop, NULL); //&armorHairTop->armorData);
    auto instanceHairLong = new RE::BGSObjectInstance(armorHairLong,  NULL); //&armorHairLong->armorData);
    auto instanceHairBeard = new RE::BGSObjectInstance(armorHairBeard,  NULL); //&armorHairBeard->armorData);

    auto equipManager = RE::ActorEquipManager::GetSingleton();

    bool anyChange = false;

    if (!isVisibleHelmetWorn || !isEquipped)
    {
        anyChange = anyChange || equipManager->UnequipObject(actor, instanceHairTop, 1, armorHairTop->equipSlot, 0, true, true, false, true, NULL);
        anyChange = anyChange || equipManager->UnequipObject(actor, instanceHairLong, 1, armorHairLong->equipSlot, 0, true, true, false, true, NULL);
        anyChange = anyChange || equipManager->UnequipObject(actor, instanceHairBeard, 1, armorHairBeard->equipSlot, 0, true, true, false, true, NULL);

        if (anyChange)
        {
            actor->Reset3D(true, 0, true, 0xC);
        }

        return isUnequipEvent != isEquipped;
    }

    bool res = true;

    if (ActorManager::WornHasKeyword(actor, setup.keywordHairTop))
    {
        bool equipSuccessful = equipManager->EquipObject(actor, *instanceHairTop, 0, 1, armorHairTop->equipSlot, true, true, false, true, true);
        res = res && equipSuccessful;
        anyChange = anyChange || equipSuccessful;
    }

    if (ActorManager::WornHasKeyword(actor, setup.keywordHairLong))
    {
        bool equipSuccessful = equipManager->EquipObject(actor, *instanceHairLong, 0, 1, armorHairLong->equipSlot, true, true, false, true, true);
        res = res && equipSuccessful;
        anyChange = anyChange || equipSuccessful;
    }

    if (ActorManager::WornHasKeyword(actor, setup.keywordHairBeard))
    {
        bool equipSuccessful = equipManager->EquipObject(actor, *instanceHairBeard, 0, 1, armorHairBeard->equipSlot, true, true, false, true, true);
        res = res && equipSuccessful;
        anyChange = anyChange || equipSuccessful;
    }

    if (anyChange)
    {
        actor->Reset3D(true, 0, true, 0xC);
    }

    return res && (isUnequipEvent != isEquipped);
}