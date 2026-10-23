/**
 * @file ProcessList.cpp
 * @brief 戦闘中のProcess登録・検索および実行処理の実装。
 */

#include "ProcessList.hpp"

#include <algorithm>

namespace simulation_process_detail
{
    // すべてのスロットから共有するデバッグ用Process引数。
    // dmg / proximity_dmg はPermill。
    const DealDMGArgs debugBAArgs = {
        Element::Void, TargetRange::Single, 1000, 15, 0, 0
    };
    const DealDMGArgs debugBSArgs = {
        Element::Void, TargetRange::Proximity, 1500, 12, 1200, 10
    };
    const DealDMGArgs debugSAArgs = {
        Element::Void, TargetRange::All, 1800, 10, 0, 0
    };
    // バトル開始時アビリティ（デバッグ用。スピード+1）。
    // 対象は自身、加算するSPD値は +1。
    // 効果ターン数は -1（常時発動型＝永続）。
    const BuffSPDFixedArgs debugAbilityArgs = {
        TargetRange::Self, 1, -1, true
    };
    // 割合型アビリティ（デバッグ用。スピード+10%）。
    // 対象は自身、割合 +10% はPermill 100 で表す。
    const BuffSPDRatioArgs debugAbilityRatioArgs = {
        TargetRange::Self, 100, -1, true
    };
    // 持続1ターンのアビリティ（デバッグ用。スピード+1）。
    // 効果ターン数を 1 にすると、保持者のターン終了時に期限切れとなる。
    const BuffSPDFixedArgs debugAbilityTimedArgs = {
        TargetRange::Self, 1, 1, false
    };
    // 対象指定版（他ユニットへ与える場合用）。対象範囲 Single。
    const BuffSPDFixedArgs debugAbilityTargetArgs = {
        TargetRange::Single, 1, 1, false
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
    const EffectCall debugAbilityEffects[] = {
        { EffectType::BuffSPDFixed, &debugAbilityArgs }
    };
    const EffectCall debugAbilityRatioEffects[] = {
        { EffectType::BuffSPDRatio, &debugAbilityRatioArgs }
    };
    const EffectCall debugAbilityTimedEffects[] = {
        { EffectType::BuffSPDFixed, &debugAbilityTimedArgs }
    };
    const EffectCall debugAbilityTargetEffects[] = {
        { EffectType::BuffSPDFixed, &debugAbilityTargetArgs }
    };

    const std::vector<ProcessDefinition> registry = {
        { DEBUG_PROCESS_BA_ID, debugBAEffects, 1 },
        { DEBUG_PROCESS_BS_ID, debugBSEffects, 1 },
        { DEBUG_PROCESS_SA_ID, debugSAEffects, 1 },
        { DEBUG_PROCESS_ABILITY_ID, debugAbilityEffects, 1 },
        { DEBUG_PROCESS_ABILITY_RATIO_ID, debugAbilityRatioEffects, 1 },
        { DEBUG_PROCESS_ABILITY_TIMED_ID, debugAbilityTimedEffects, 1 },
        { DEBUG_PROCESS_ABILITY_TARGET_ID, debugAbilityTargetEffects, 1 }
    };
}

/**
 * @brief 登録済みのProcess定義リストを取得します。
 */
const std::vector<ProcessDefinition>& GetProcessList()
{
    return simulation_process_detail::registry;
}

/**
 * @brief 登録済みProcessの総数を取得します。
 */
std::size_t GetRegisteredProcessCount()
{
    return simulation_process_detail::registry.size();
}

/**
 * @brief 指定したProcessIdに対応する定義を取得します。
 */
const ProcessDefinition* GetProcessDefinition(ProcessId id)
{
    if (id == 0)
    {
        return nullptr;
    }

    const auto& registry = simulation_process_detail::registry;
    const auto it = std::find_if(registry.begin(), registry.end(),
        [id](const ProcessDefinition& process)
        {
            return process.id == id;
        });

    return it == registry.end() ? nullptr : &(*it);
}

/**
 * @brief 指定したProcess定義を実行します。
 */
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

/**
 * @brief 指定したProcessIdに対応するProcessを検索して実行します。
 */
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
