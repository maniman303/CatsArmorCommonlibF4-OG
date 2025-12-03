#include "PerkDistributor.h"
#include "FormUtil.h"
#include "Setup.h"
#include "Files.h"

namespace PerkDistributor
{
    bool IsRaceOfHumanOrigin(RE::TESRace* race)
    {
        if (race == NULL)
        {
            return false;
        }

        auto humanRace = FormUtil::GetFormFromMod("Fallout4.esm", 0x00013746);
        if (race == humanRace)
        {
            return true;
        }

        auto armorRace = race->armorParentRace;

        return IsRaceOfHumanOrigin(armorRace);
    }

    bool IsNpcValid(RE::TESNPC* npc, bool excludePlayer)
    {
        if (!Files::IsFilePresent() || !Setup::IsInitialized())
        {
            return false;
        }

        if (npc == NULL)
        {
            // REX::INFO("Npc is null");
            return false;
        }

        if ((npc->IsPlayer() && excludePlayer) || npc->IsDeleted() || npc->IsDisabled())
        {
            // REX::INFO("Npc doesn't match criteria... [{}]", npc->GetFullName());
            return false;
        }

        auto race = npc->GetFormRace();
        if (!IsRaceOfHumanOrigin(race))
        {
            // REX::INFO("Npc is not human... [{}]", npc->GetFullName());
            return false;
        }

        return true;
    }

    bool IsNpcValid(RE::TESNPC* npc)
    {
        return IsNpcValid(npc, true);
    }

    bool IsSpellIncluded(RE::SpellItem* spell, RE::TESSpellList::SpellData* spellList)
    {
        for (uint32_t i = 0; i < spellList->numSpells; i++)
        {
            auto iter = spellList->spells[i];

            if (iter == spell)
            {
                return true;
            }
        }

        return false;
    }

    bool TryProcessNpc(RE::TESNPC* npc)
    {
        if (!IsNpcValid(npc))
        {
            return false;
        }

        // REX::INFO("Found human! [{}]", npc->GetFullName());

        auto spell = Setup::GetSpell();
        auto perk = Setup::GetPerk();

        if (spell == NULL || perk == NULL)
        {
            return false;
        }

        auto perkIndex = npc->GetPerkIndex(perk);
        if (perkIndex.has_value() && perkIndex.value() > 0)
        {
            return false;
        }

        bool res = true;

        if (npc->perks)
        {
            if (!npc->AddPerk(perk, 1))
            {
                REX::WARN(std::format("Failed to add perk to [{}].", npc->GetFullName()));
                res = false;
            }
        }

        auto spellList = npc->GetSpellList();
        if (spellList)
        {
            if (!npc->AddSpell(spell) && !IsSpellIncluded(spell, spellList))
            {
                REX::WARN(std::format("Failed to add spell to [{}].", npc->GetFullName()));
                res = false;
            }
        }

        if (res)
        {
            // REX::INFO(std::format("Added spell and perk to [{}]", npc->GetFullName()));
        }

        return res;
    }

    bool TryRevertActor(RE::Actor* actor)
    {
        auto npc = actor->GetNPC();

        if (!IsNpcValid(npc))
        {
            return false;
        }

        auto spell = Setup::GetSpell();
        auto perk = Setup::GetPerk();

        if (spell == NULL || perk == NULL)
        {
            return false;
        }

        auto perkIndex = npc->GetPerkIndex(perk);
        if (perkIndex.has_value() && perkIndex.value() > 0)
        {
            actor->RemovePerk(perk);
        }

        // npc->GetSpellList()->RemoveSpell(spell);

        // REX::INFO("Removed spell and perk from [{}]", npc->GetFullName());

        return true;
    }

    void ProcessMemoryActors()
    {
        auto processLists = RE::ProcessLists::GetSingleton();

        int processedNpcs = 0;

        for (const auto& actorHandle : processLists->lowActorHandles)
        {
            const auto& actor = actorHandle.get();
            if (actor == NULL)
            {
                continue;
            }

            auto npc = actor->GetNPC();
            if (npc == NULL)
            {
                continue;
            }

            if (TryProcessNpc(npc))
            {
                processedNpcs++;
            }
        }

        REX::INFO(std::format("Processed {} npcs.", processedNpcs));
    }
}