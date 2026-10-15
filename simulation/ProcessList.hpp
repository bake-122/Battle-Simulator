#ifndef PROCESS_LIST_HPP
#define PROCESS_LIST_HPP

#include <cstddef>
#include <cstdint>
#include <vector>

#include "EffectList.hpp"

// Process IDはEffectTypeとは独立した32-bit ID。
// 約4000件の手動登録を想定し、Effect IDとは別の名前空間で管理する。
using ProcessId = std::uint32_t;

// デバッグ用Process ID。正式なProcess ID (KiokuID * 1000 + ...) と
// 衝突しないよう、正式データで使用しない低いIDを割り当てる。
inline constexpr ProcessId DEBUG_PROCESS_BA_ID = 1;
inline constexpr ProcessId DEBUG_PROCESS_BS_ID = 2;
inline constexpr ProcessId DEBUG_PROCESS_SA_ID = 3;

struct ProcessDefinition
{
    ProcessId id = 0;                    // 0は未使用IDとして予約
    const EffectCall* effects = nullptr; // Process内で実行するEffect列
    std::size_t effectCount = 0;
};

// 手動登録されたProcess一覧。戻り値の参照はプログラム終了まで有効。
const std::vector<ProcessDefinition>& GetProcessList();
std::size_t GetRegisteredProcessCount();

// IDに対応するProcessを返す。未登録ならnullptr。
const ProcessDefinition* GetProcessDefinition(ProcessId id);

// Process定義またはIDを指定して実行する。
void ExecuteProcess(const ProcessDefinition& process, const EffectContext& context);
bool ExecuteProcess(ProcessId id, const EffectContext& context);

#endif // PROCESS_LIST_HPP
