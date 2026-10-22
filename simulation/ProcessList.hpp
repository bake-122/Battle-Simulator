/**
 * @file ProcessList.hpp
 * @brief 戦闘中のProcess（Effect列の組み合わせ）定義および実行インターフェースを提供するヘッダーファイル。
 */

#ifndef PROCESS_LIST_HPP
#define PROCESS_LIST_HPP

#include <cstddef>
#include <cstdint>
#include <vector>

#include "EffectList.hpp"

/**
 * @brief Processの一意な識別子型（32ビット符号なし整数）。
 */
using ProcessId = std::uint32_t;

/**
 * @brief 通常攻撃（BA）用のデバッグProcess ID。
 */
inline constexpr ProcessId DEBUG_PROCESS_BA_ID = 1;

/**
 * @brief バトルスキル（BS）用のデバッグProcess ID。
 */
inline constexpr ProcessId DEBUG_PROCESS_BS_ID = 2;

/**
 * @brief 必殺技（SA）用のデバッグProcess ID。
 */
inline constexpr ProcessId DEBUG_PROCESS_SA_ID = 3;

/**
 * @brief アビリティ効果のデバッグProcess ID。
 */
inline constexpr ProcessId DEBUG_PROCESS_ABILITY_ID = 4;

/**
 * @brief Processの定義構造体。
 */
struct ProcessDefinition
{
    ProcessId id = 0;                    /**< Process ID（0は未使用ID） */
    const EffectCall* effects = nullptr; /**< Process内で実行するEffect列配列ポインタ */
    std::size_t effectCount = 0;         /**< 含まれるEffect数 */
};

/**
 * @brief 登録済みのProcess定義リストを取得します。
 * @return ProcessDefinitionリストへのconst参照。
 */
const std::vector<ProcessDefinition>& GetProcessList();

/**
 * @brief 登録済みProcessの総数を取得します。
 * @return 登録数。
 */
std::size_t GetRegisteredProcessCount();

/**
 * @brief 指定したProcessIdに対応する定義を取得します。
 * @param id 検索するProcessId。
 * @return ProcessDefinitionへのポインタ。未登録時はnullptr。
 */
const ProcessDefinition* GetProcessDefinition(ProcessId id);

/**
 * @brief 指定したProcess定義を実行します。
 * @param process 実行するProcess定義。
 * @param context Effect実行コンテキスト。
 */
void ExecuteProcess(const ProcessDefinition& process, const EffectContext& context);

/**
 * @brief 指定したProcessIdに対応するProcessを検索して実行します。
 * @param id 実行するProcessId。
 * @param context Effect実行コンテキスト。
 * @return 実行に成功した場合はtrue、未登録等で失敗した場合はfalse。
 */
bool ExecuteProcess(ProcessId id, const EffectContext& context);

#endif // PROCESS_LIST_HPP
