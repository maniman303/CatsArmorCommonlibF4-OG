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

bool ActorManager::WornHasKeyword(RE::Actor* actor, RE::BGSKeyword* keyword, const RE::TBO_InstanceData* instance)
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

            auto instanceData = itemData.GetInstanceData(i);
            if (instance != NULL && instance != instanceData)
            {
                continue;
            }

            if (armor->HasKeyword(keyword))
            {
                return true;
            }

            if (instanceData == NULL)
            {
                // REX::INFO("Continued.");
                continue;
            }

            // REX::INFO("Has instance data.");

            auto keywordData = instanceData->GetKeywordData();
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

        RE::TBO_InstanceData* expectedInstanceData = NULL;
        if (instance->instanceData)
        {
            // REX::INFO("Using instance data in equip validation.");
            expectedInstanceData = instance->instanceData.get();
        }

        for (uint32_t i = 0; i < CountStacks(itemData); i++)
        {
            auto stack = itemData.GetStackByID(i);
            if (stack == NULL)
            {
                continue;
            }

            if (itemData.GetInstanceData(i) == expectedInstanceData)
            {
                return stack->IsEquipped();
            }

            if (expectedInstanceData == NULL && itemData.object == instance->object && stack->IsEquipped())
            {
                return true;
            }
        }
    }

    return false;
}

bool ActorManager::UnequipItem(RE::Actor* actor, RE::TESObjectARMO* armor)
{
    if (actor == NULL || armor == NULL || actor->inventoryList == NULL)
    {
        return false;
    }

    auto equipManager = RE::ActorEquipManager::GetSingleton();

    for (auto itemData : actor->inventoryList->data)
    {
        auto object = itemData.object;
        if (object == NULL)
        {
            continue;
        }

        if (object != armor)
        {
            continue;
        }

        if (itemData.GetCount() <= 0)
        {
            continue;
        }

        for (uint32_t i = 0; i < CountStacks(itemData); i++)
        {
            auto stack = itemData.GetStackByID(i);
            if (!stack->IsEquipped())
            {
                continue;
            }

            auto instanceData = itemData.GetInstanceData(i);
            auto instance = new RE::BGSObjectInstance(armor, instanceData);
            equipManager->UnequipObject(actor, instance, 1, armor->equipSlot, i, false, true, false, true, NULL);
            return true;
        }
    }

    return true;
}

bool ActorManager::EquipItem(RE::Actor* actor, RE::TESObjectARMO* armor)
{
    if (actor == NULL || armor == NULL || actor->inventoryList == NULL)
    {
        return false;
    }

    auto itemCount = actor->GetInventoryObjectCount(armor);
    // if (itemCount > 0)
    // {
    //     RE::TESObjectREFR::RemoveItemData removeData(armor, 1);
    //     actor->RemoveItem(removeData);
    //     itemCount = 0; 
    // }

    if (itemCount == 0)
    {
        auto equipIndex = RE::BGSEquipIndex();
        equipIndex.index = 0;

        actor->inventoryList->rwLock.lock_write();

        actor->AddInventoryItem(armor->As<RE::TESBoundObject>(), NULL, 1, NULL, NULL, NULL);
        // actor->inventoryList->AddItem2(armor->As<RE::TESBoundObject>(), 1, new RE::ExtraDataList(), 0);

        actor->inventoryList->rwLock.unlock_write();
    }

    auto equipManager = RE::ActorEquipManager::GetSingleton();
    for (auto itemData : actor->inventoryList->data)
    {
        auto object = itemData.object;
        if (object == NULL)
        {
            continue;
        }

        if (itemData.GetCount() <= 0)
        {
            continue;
        }

        if (object != armor)
        {
            continue;
        }

        if (CountStacks(itemData) > 0)
        {
            auto stack = itemData.GetStackByID(0);
            if (stack->IsEquipped())
            {
                // REX::INFO("Item already equipped.");
                return true;
            }

            auto instanceData = itemData.GetInstanceData(0);
            auto instance = new RE::BGSObjectInstance(armor, instanceData);
            return equipManager->EquipObject(actor, *instance, 0, 1, armor->equipSlot, false, true, false, true, false);
        }
    }

    REX::ERROR("Couldn't find armor in actors inventory to equip.");
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
        return true;
    }

    // REX::INFO(std::format("PROCESSING... Is visible: {0}, is unequip: {1}, is equipped: {2}.", isVisibleHelmetWorn, isUnequipEvent, isEquipped));

    auto armorHairTop = setup.armorHairTop;
    auto armorHairLong = setup.armorHairLong;
    auto armorHairBeard = setup.armorHairBeard;

    uint8_t anyChange = 0;

    if (!isVisibleHelmetWorn || !isEquipped)
    {
        anyChange += ActorManager::UnequipItem(actor, armorHairTop) ? 1 : 0;
        anyChange += ActorManager::UnequipItem(actor, armorHairLong) ? 1 : 0;
        anyChange += ActorManager::UnequipItem(actor, armorHairBeard) ? 1 : 0;

        if (anyChange > 0)
        {
            // REX::INFO("Should updated unequipped items.");
            actor->HandleItemEquip(false);
        }

        return isUnequipEvent != isEquipped;
    }

    RE::TBO_InstanceData* armorInstance = NULL;
    if (armor != NULL && armor->instanceData)
    {
        armorInstance = armor->instanceData.get();
    }

    bool res = true;

    if (ActorManager::WornHasKeyword(actor, setup.keywordHairTop, armorInstance) && !isUnequipEvent)
    {
        bool equipSuccessful = ActorManager::EquipItem(actor, armorHairTop);
        res = res && equipSuccessful;
        anyChange += equipSuccessful ? 1 : 0;
    }
    else
    {
        bool unequipSuccessful = ActorManager::UnequipItem(actor, armorHairTop);
        res = res && unequipSuccessful;
        anyChange += unequipSuccessful ? 1 : 0;
    }

    if (ActorManager::WornHasKeyword(actor, setup.keywordHairLong, armorInstance) && !isUnequipEvent)
    {
        bool equipSuccessful = ActorManager::EquipItem(actor, armorHairLong);
        res = res && equipSuccessful;
        anyChange += equipSuccessful ? 1 : 0;
    }
    else
    {
        bool unequipSuccessful = ActorManager::UnequipItem(actor, armorHairLong);
        res = res && unequipSuccessful;
        anyChange += unequipSuccessful ? 1 : 0;
    }

    if (ActorManager::WornHasKeyword(actor, setup.keywordHairBeard, armorInstance) && !isUnequipEvent)
    {
        bool equipSuccessful = ActorManager::EquipItem(actor, armorHairBeard);
        res = res && equipSuccessful;
        anyChange += equipSuccessful ? 1 : 0;
    }
    else
    {
        bool unequipSuccessful = ActorManager::UnequipItem(actor, armorHairBeard);
        res = res && unequipSuccessful;
        anyChange += unequipSuccessful ? 1 : 0;
    }

    if (anyChange > 0)
    {
        // REX::INFO(std::format("Should updated equipped items with change [{0}].", anyChange));
        actor->HandleItemEquip(false);
    }

    return res && !isUnequipEvent;
}