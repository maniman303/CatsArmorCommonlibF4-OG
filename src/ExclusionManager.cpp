#include "ExclusionManager.h"
#include <unordered_set>
#include "Files.h"
#include "FormUtil.h"
#include <json/json.h>

std::unordered_set<RE::TESForm*> Exclusions;

int ProcessJson(Json::Value json)
{
    if (json.empty())
    {
        return 0;
    }

    auto formIds = FormUtil::GetFormIdsFromJson(json);
    auto forms = FormUtil::GetFormsFromList(formIds);
    int count = 0;

    for (auto& form : forms)
    {
        if (Exclusions.contains(form))
        {
            continue;
        }

        Exclusions.insert(form);

        count++;
    }

    return count;
}

void ExclusionManager::Initialize()
{
    Exclusions.clear();

    auto exclusionFiles = Files::GetPluginFiles("Exclusion");

    int count = 0;

    for (auto& file : exclusionFiles)
    {
        auto& path = file.path();

		Json::Value modJson;
		std::ifstream modFile;
		
		try
		{
			modFile.open(path);
			modFile >> modJson;
			modFile.close();
		}
		catch (...)
		{
			REX::ERROR(std::format("Invalid json '{0}'.", path.string()));
			continue;
		}

        count += ProcessJson(modJson);
    }

    REX::INFO(std::format("Processed {0} exclusions.", count));
}

bool ExclusionManager::Contains(RE::TESForm* form)
{
    return Exclusions.contains(form);
}