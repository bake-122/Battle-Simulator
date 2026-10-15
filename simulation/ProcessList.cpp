#include "ProcessList.hpp"

#include <algorithm>

namespace exedra_process_detail
{
    // すべてのスロットから共有するデバッグ用Process引数。
    // dmg / proximity_dmg はパーミル単位。コンソールには従来どおりパーセントで表示する。
    const DealDMGArgs debugBAArgs = {
        Element::Void, TargetRange::Single, 1000, 15, 0, 0
    };
    const DealDMGArgs debugBSArgs = {
        Element::Void, TargetRange::Proximity, 1500, 12, 1200, 10
    };
    const DealDMGArgs debugSAArgs = {
        Element::Void, TargetRange::All, 1800, 10, 0, 0
    };

    const EffectCall debugBAEffects[] = {
        { EffectType::DealDMG, &debugBAArgs }
    };
    const EffectCall debugBSEffects[] = {
        { EffectType::DealDMG, &debugBSArgs }
    };
    const EffectCall debugSAEffects[] = {
        { EffectType::DealDMG, &debugSAArgs }
    };

    const std::vector<ProcessDefinition> registry = {
        { DEBUG_PROCESS_BA_ID, debugBAEffects, 1 },
        { DEBUG_PROCESS_BS_ID, debugBSEffects, 1 },
        { DEBUG_PROCESS_SA_ID, debugSAEffects, 1 }
    };
}

const std::vector<ProcessDefinition>& GetProcessList()
{
    return exedra_process_detail::registry;
}

std::size_t GetRegisteredProcessCount()
{
    return exedra_process_detail::registry.size();
}

const ProcessDefinition* GetProcessDefinition(ProcessId id)
{
    if (id == 0)
    {
        return nullptr;
    }

    const auto& registry = exedra_process_detail::registry;
    const auto it = std::find_if(registry.begin(), registry.end(),
        [id](const ProcessDefinition& process)
        {
            return process.id == id;
        });

    return it == registry.end() ? nullptr : &(*it);
}

void ExecuteProcess(const ProcessDefinition& process, const EffectContext& context)
{
    if (process.effects == nullptr)
    {
        return;
    }

    for (std::size_t i = 0; i < process.effectCount; ++i)
    {
        ExecuteEffect(process.effects[i], context);
    }
}

bool ExecuteProcess(ProcessId id, const EffectContext& context)
{
    const ProcessDefinition* process = GetProcessDefinition(id);
    if (process == nullptr)
    {
        return false;
    }

    ExecuteProcess(*process, context);
    return true;
}
