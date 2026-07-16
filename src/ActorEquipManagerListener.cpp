#include <excpt.h>
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

        uint32_t hairTopMask = 1;
		uint32_t hairLongMask = 2;
		uint32_t hairBeardMask = 1 << 18;
		uint32_t headbandMask = 1 << 16;

        uint32_t mask = hairTopMask | hairLongMask | hairBeardMask | headbandMask;

        auto bipedSlots = armor->bipedModelData.bipedObjectSlots;
        if ((bipedSlots & mask) == 0)
        {
            return NULL;
        }

        return armor;
    }

    struct PapyrusEventData
    {
        RE::BSScript::IVirtualMachine* vm;
        uint32_t formId;
    };

    static uint32_t GetSafePapyrusFormId(PapyrusEventData* dataPtr)
    {
        __try
        {
            return dataPtr->formId;
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            return 0;
        }
    }

    static void RunHeadgearPapyrusEvent(uint64_t handle, const char* scriptName, const char* callbackName, void* dataPtr)
    {
        if (dataPtr == NULL) {
            REX::WARN("Event distributed for a null data.");
            return;
        }
        
        auto d = static_cast<PapyrusEventData*>(dataPtr);
        uint32_t actorId = GetSafePapyrusFormId(d);
        if (actorId == 0) {
            REX::ERROR("Could not retrieve id of the papyrus event actor.");
            return;
        }
        
        RE::BSFixedString sn;
        if (scriptName != NULL) {
            sn = scriptName;
        }

        RE::BSFixedString cn;
        if (callbackName != NULL) {
            cn = callbackName;
        }

        d->vm->DispatchMethodCall(handle, sn, cn, NULL, actorId);
        delete d;
    }

    static uint32_t GetNpcFormIdFromActor(RE::TESObjectREFR* objectRef)
    {
        if (objectRef == NULL || !objectRef->Is<RE::Actor>())
        {
            return 0;
        }

        auto actorRef = objectRef->As<RE::Actor>();
        auto npc = actorRef->GetNPC();
        if (npc == NULL)
        {
            return 0;
        }

        return npc->GetFormID();
    }

    static bool AddSafeKeyword(RE::TESObjectREFR* objectRef, RE::BGSKeyword* kywd)
    {
        __try
        {
            objectRef->AddKeyword(kywd);
            return true;
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            return false;
        }
    }

    void SendHeadgearPapyrusEvent(RE::TESObjectREFR* objectRef)
	{
        if (objectRef == NULL)
        {
            return;
        }

        auto kywd = Setup::GetActorKeyword();
        if (kywd == NULL)
        {
            return;
        }

        uint32_t actorId = GetNpcFormIdFromActor(objectRef);
        if (actorId == 0)
        {
            REX::ERROR("Could not retrieve actor id during papyrus event processing.");
            return;
        }

        if (!AddSafeKeyword(objectRef, kywd))
        {
            REX::ERROR(std::format("Could not add keyword to actor with id [0x{:08X}].", actorId));
            return;
        }

		auto eventData = new PapyrusEventData();
		auto const papyrus = F4SE::GetPapyrusInterface();
		auto* vm = RE::GameVM::GetSingleton()->GetVM().get();

		eventData->vm = vm;
		eventData->formId = actorId;

		papyrus->GetExternalEventRegistrations("HeadgearEquipEvent", eventData, [](uint64_t handle, const char* scriptName, const char* callbackName, void* dataPtr) {
            RunHeadgearPapyrusEvent(handle, scriptName, callbackName, dataPtr);
		});
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

        // REX::INFO("Item not null");

        if (!PerkDistributor::IsNpcValid(npc, false))
        {
            return RE::BSEventNotifyControl::kContinue;
        }

        // REX::INFO("Npc is valid");

        uint32_t actorId = GetNpcFormIdFromActor(actor);
        if (actorId == 0)
        {
            REX::ERROR("Could not retrieve actor id during equip event processing.");
            return RE::BSEventNotifyControl::kContinue;
        }

        // REX::INFO(std::format("Processing actor [0x{:08X}].", actorId));
        
        if (!ActorManager::ProcessHairStubs(actor, itemInstance, aEvent.changeType.get() == RE::ActorEquipManagerEvent::Type::kUnequip))
        {
            // REX::INFO(std::format("Send headgear event for actor [0x{:08X}].", actorId));

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