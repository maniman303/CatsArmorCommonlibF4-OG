#include "ActorEquipManagerListener.h"
#include "PerkDistributor.h"
#include "PapyrusUtil.h"
#include "Setup.h"
#include "Files.h"
#include "ActorManager.h"

class ActorEquipManagerSink : public RE::BSTEventSink<RE::ActorEquipManagerEvent::Event>
{
public:
    static ActorEquipManagerSink* GetSingleton()
    {
        static ActorEquipManagerSink singleton;
        return std::addressof(singleton);
    }

private:
    RE::TESObjectARMO* TryGetHeadgear(RE::TESForm* item)
    {
        if (item == NULL)
        {
            return NULL;
        }

        if (!item->Is(RE::ENUM_FORM_ID::kARMO))
        {
            return NULL;
        }

        auto armor = item->As<RE::TESObjectARMO>();

        if ((armor->formFlags & 4) != 0)
		{
			return NULL;
		}

		uint32_t headbandMask = 1 << 16;

        uint32_t mask = headbandMask;

        auto bipedSlots = armor->bipedModelData.bipedObjectSlots;
        if ((bipedSlots & mask) == 0)
        {
            return NULL;
        }

        return armor;
    }

    void SendHeadgearPapyrusEvent(RE::TESObjectREFR* objectRef)
	{
		struct PapyrusEventData
		{
			RE::BSScript::IVirtualMachine* vm;
			uint32_t formId;
            RE::TESObjectREFR* objectRef;
		};

        if (objectRef == NULL)
        {
            return;
        }

        auto kywd = Setup::GetActorKeyword();
        if (kywd == NULL)
        {
            return;
        }

        objectRef->AddKeyword(kywd);

		PapyrusEventData eventData;
		auto const papyrus = F4SE::GetPapyrusInterface();
		auto* vm = RE::GameVM::GetSingleton()->GetVM().get();

		eventData.vm = vm;
		eventData.formId = objectRef->GetFormID();
        eventData.objectRef = objectRef;

		papyrus->GetExternalEventRegistrations("HeadgearEquipEvent", &eventData, [](uint64_t handle, const char* scriptName, const char* callbackName, void* dataPtr) {
			PapyrusEventData* d = static_cast<PapyrusEventData*>(dataPtr);
			d->vm->DispatchMethodCall<uint32_t>(handle, scriptName, callbackName, NULL, d->formId);
		});
	}

    bool QuickProcessing(RE::Actor* actor, bool isUnequip)
    {
        auto setup = Setup::GetForms("headgear");

        bool isVisibleHelmetWorn = ActorManager::WornHasKeyword(actor, setup.keyword) &&
            !ActorManager::WornHasKeyword(actor, setup.keywordHidden);

        if (isVisibleHelmetWorn == isUnequip && !isUnequip)
        {
            return false;
        }

        auto instanceHairTop = RE::BGSObjectInstance(setup.armorHairTop, NULL);
        auto instanceHairLong = RE::BGSObjectInstance(setup.armorHairLong, NULL);
        auto instanceHairBeard = RE::BGSObjectInstance(setup.armorHairBeard, NULL);

        auto equipManager = RE::ActorEquipManager::GetSingleton();

        if (!isVisibleHelmetWorn || isUnequip)
        {
            equipManager->UnequipObject(actor, &instanceHairTop, 1, NULL, 0, true, true, false, true, NULL);
            equipManager->UnequipObject(actor, &instanceHairLong, 1, NULL, 0, true, true, false, true, NULL);
            equipManager->UnequipObject(actor, &instanceHairBeard, 1, NULL, 0, true, true, false, true, NULL);

            return !isUnequip;
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

        return res;
    }

    RE::BSEventNotifyControl ProcessEvent(const RE::ActorEquipManagerEvent::Event& aEvent, RE::BSTEventSource<RE::ActorEquipManagerEvent::Event>*) override
    {
        if (!Files::IsFilePresent() || !Setup::IsInitialized())
        {
            return RE::BSEventNotifyControl::kContinue;
        }

        auto actor = aEvent.actorAffected;
        auto itemInstance = aEvent.itemAffected;

        if (actor == NULL || itemInstance == NULL)
        {
            return RE::BSEventNotifyControl::kContinue;
        }

        auto npc = actor->GetNPC();
        if (npc == NULL)
        {
            return RE::BSEventNotifyControl::kContinue;
        }

        auto item = TryGetHeadgear(itemInstance->object);
        if (item == NULL)
        {
            return RE::BSEventNotifyControl::kContinue;
        }

        if (!PerkDistributor::IsNpcValid(npc, false))
        {
            return RE::BSEventNotifyControl::kContinue;
        }

        // REX::INFO("Send headgear event");

        if (!QuickProcessing(actor, aEvent.isUnequip > 0))
        {
            SendHeadgearPapyrusEvent(actor);
        }

        return RE::BSEventNotifyControl::kContinue;
    }
};

void ActorEquipManagerListener::Register()
{
    auto mgr = RE::ActorEquipManager::GetSingleton();
    if (mgr == NULL)
    {
        REX::WARN("ActorEquipManager is null.");

        return;
    }

    auto sink = ActorEquipManagerSink::GetSingleton();
    if (sink == NULL)
    {
        REX::WARN("ActorEquipManagerSink is null.");

        return;
    }

    mgr->UnregisterSink(sink);
    mgr->RegisterSink(sink);

    REX::INFO("ActorEquipManager event listener registered.");
}