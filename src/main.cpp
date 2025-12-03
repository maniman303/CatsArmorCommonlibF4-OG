#include <json/json.h>
#include <iostream>
#include <fstream>
#include <bitset>
#include "FormUtil.h"
#include "Workaround.h"
#include "Setup.h"
#include "Hooks.h"
#include "Files.h"
#include "PapyrusUtil.h"
#include "ArmorProcessor.h"
#include "HeadgearProcessor.h"
#include "PerkDistributor.h"
#include "ActorEquipManagerListener.h"

extern "C" DLLEXPORT bool F4SEAPI F4SEPlugin_Query(const F4SE::QueryInterface* a_f4se, F4SE::PluginInfo* a_info)
{
#ifndef NDEBUG
	auto sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
#else
	auto path = logger::log_directory();
	if (!path) {
		return false;
	}

	*path /= fmt::format(FMT_STRING("{}.log"), Version::PROJECT);
	auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
#endif

	auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));

#ifndef NDEBUG
	log->set_level(spdlog::level::trace);
#else
	log->set_level(spdlog::level::info);
	log->flush_on(spdlog::level::info);
#endif

	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("[%H:%M:%S] %^%l%$: %v"s);

	logger::info("{} v{}", Version::PROJECT, Version::NAME);

	a_info->infoVersion = F4SE::PluginInfo::kVersion;
	a_info->name = Version::PROJECT.data();
	a_info->version = Version::MAJOR;

	if (a_f4se->IsEditor()) {
		logger::critical("loaded in editor");
		return false;
	}

	const auto ver = a_f4se->RuntimeVersion();
	if (ver < F4SE::RUNTIME_1_10_162) {
		logger::critical("unsupported runtime v{}", ver.string());
		return false;
	}

	return true;
}

void OnMessage(F4SE::MessagingInterface::Message* message)
{
	if (message->type == F4SE::MessagingInterface::kGameDataReady)
	{
		if (!Files::VerifyPaths())
		{
			REX::ERROR("The main plugin directory does not exists.");
			return;
		}

		Files::PrepareDirectories();

		if (!Files::VerifyCatsPlugin())
		{
			REX::ERROR("Plugin 'Cats Armor.esl' is missing.");
			return;
		}

		if (!Setup::Initialize())
		{
			REX::ERROR("Incomplete setup.");
			return;
		}

		Hooks::Install();

		ArmorProcessor::ProcessArmorFiles();
		HeadgearProcessor::ProcessHeadgearFiles();

		ActorEquipManagerListener::Register();

		PerkDistributor::ProcessMemoryActors();

		REX::INFO("Finished pre-game processing.");
	}
}

extern "C" DLLEXPORT bool F4SEAPI F4SEPlugin_Load(const F4SE::LoadInterface* a_f4se)
{
	F4SE::Init(a_f4se);

	F4SE::GetPapyrusInterface()->Register(PapyrusUtil::BindFunctions);

	F4SE::GetMessagingInterface()->RegisterListener(OnMessage);

	REX::INFO("Finished initialization.");

	return true;
}
