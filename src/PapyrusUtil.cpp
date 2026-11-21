#include "PapyrusUtil.h"

namespace PapyrusUtil
{
	std::vector<int> GetArmorBipedSlots(std::monostate, RE::TESObjectARMO* armor)
	{
		std::vector<int> result;

		if (armor == NULL) {
			return result;
		}

		auto slotsBits = armor->bipedModelData.bipedObjectSlots;

		for (int i = 0; i < 44; i++) {
			if ((slotsBits & 1) > 0) {
				result.push_back(i);
			}

			slotsBits = slotsBits >> 1;
		}

		return result;
	}

	void LogScript(std::monostate, RE::BSFixedString message)
	{
		if (message.empty())
		{
			return;
		}

		std::string text = message.c_str();
		if (text == "")
		{
			return;
		}

		REX::INFO(std::format("[Script] {0}", text));
	}

	void Notification(std::monostate, RE::BSFixedString message)
	{
		if (message.empty())
		{
			return;
		}

		std::string text = message.c_str();
		if (text == "")
		{
			return;
		}
		
		auto vm = RE::GameVM::GetSingleton()->GetVM().get();
		auto scrapFunc = (Papyrus::FunctionArgs{ vm, message }).get();

		vm->DispatchStaticCall("Debug", "Notification", scrapFunc, NULL);
	}

	RE::BSFixedString StringReplace(std::monostate, RE::BSFixedString input, RE::BSFixedString toFind, RE::BSFixedString toReplace)
	{
		std::string result = input.c_str();
		const std::string find = toFind.c_str();
		const std::string replace = toReplace.c_str();

		if (find.empty() || find.empty() || replace.empty()) {
			return input;
		}

		std::size_t pos = 0;
		while ((pos = result.find(find, pos)) != std::string::npos) {
			result.replace(pos, find.length(), replace);
			pos += replace.length();
		}

		return RE::BSFixedString(result.c_str());
	}

	RE::BSFixedString GetMessageShortTitle(std::monostate, RE::BGSMessage* message)
	{
		if (message == NULL) {
			return RE::BSFixedString();
		}

		std::string result = message->shortName.c_str();
		if (result.empty()) {
			return RE::BSFixedString();
		}

		REX::INFO(std::format("[Script] GetMessageShortTitle: {0} {1}", result, message->fullName.c_str()));

		return RE::BSFixedString(result.c_str());
	}

	RE::BSFixedString GetMessageTitle(std::monostate, RE::BGSMessage* message)
	{
		if (message == NULL) {
			return RE::BSFixedString();
		}

		std::string result = message->fullName.c_str();
		if (result.empty()) {
			return RE::BSFixedString();
		}

		return RE::BSFixedString(result.c_str());
	}

	bool BindFunctions(RE::BSScript::IVirtualMachine* vm)
	{
		vm->BindNativeMethod("CATS:ScriptExtender", "GetArmorBipedSlots", PapyrusUtil::GetArmorBipedSlots, true);
		vm->BindNativeMethod("CATS:ScriptExtender", "Trace", PapyrusUtil::LogScript, true);
		vm->BindNativeMethod("CATS:ScriptExtender", "Notification", PapyrusUtil::Notification);
		vm->BindNativeMethod("CATS:ScriptExtender", "StringReplace", PapyrusUtil::StringReplace, true);
		vm->BindNativeMethod("CATS:ScriptExtender", "GetMessageShortTitle", PapyrusUtil::GetMessageShortTitle, true);
		vm->BindNativeMethod("CATS:ScriptExtender", "GetMessageTitle", PapyrusUtil::GetMessageTitle, true);

		return true;
	}
}
