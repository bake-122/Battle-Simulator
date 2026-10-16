/**
 * @file simulationLogic.hpp
 * @brief 戦闘シミュレーションのロジック・状態管理・タイムライン・履歴制御を担当するヘッダーファイル。
 */

#ifndef SIMULATION_LOGIC_HPP
#define SIMULATION_LOGIC_HPP

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "../unit_types.h"
#include "EffectList.hpp"
#include "ProcessList.hpp"

/**
 * @brief 戦闘中における各キオクの動的ステータス詳細。
 */
struct UnitStatDetails
{
    Character character{};   /**< キオクの静的データ */
    int hp = 0;              /**< 現在HP */
    int maxHP = 0;           /**< 戦闘開始時の最大HP */
    int atk = 0;             /**< 攻撃力 */
    int def = 0;             /**< 防御力 */
    int currentMP = 0;       /**< 現在MP */
    int currentBP = 0;       /**< BPを使用するキオク用。現時点ではゲージ処理未実装。 */
    int magicLevel = 140;    /**< 魔力解放レベル */
    float av = 0.0f;         /**< Action Value（行動値） */
};

/**
 * @brief 戦闘全体のチーム状態（味方・敵・共有SP）。
 */
struct TeamState
{
    std::array<UnitStatDetails, 5> ally{};   /**< 味方5枠 */
    std::array<UnitStatDetails, 5> enemy{};  /**< 敵5枠 */
    int sp = 5;                              /**< パーティ共有SP（初期値5、範囲0～6） */
};

/**
 * @brief タイムライン上の1ノードを表す構造体。
 */
struct TimelineNode
{
    int team;          /**< チーム（0 = enemy, 1 = ally） */
    int slotIndex;     /**< スロットインデックス（0～4） */
    int tp;            /**< ターン優先度（enemy: 1～5, ally: 6～10） */
    int id;            /**< キオクID */
    const char* name;  /**< キオク名 */
    int spd;           /**< 速度 */
    float av;          /**< 行動値（Action Value） */
};

/**
 * @brief 行動スキル種別。
 */
enum class SkillType : std::uint8_t
{
    SA = 0,       /**< Special Attack（必殺技） */
    BS = 1,       /**< Battle Skill */
    BA = 2,       /**< Basic Attack（通常攻撃） */
    Ability = 3   /**< アビリティ */
};

/**
 * @brief 現在のチーム状態への参照を取得します。
 * @return チーム状態へのconst参照。
 */
const TeamState& GetSimulationTeamState();

/**
 * @brief ロジック側のチーム状態を設定・初期化します。
 * @param state 設定するチーム状態。
 */
void SetSimulationTeamStateLogic(const TeamState& state);

/**
 * @brief 現在のタイムラインを取得します。
 * @return タイムラインノード配列へのconst参照。
 */
const std::vector<TimelineNode>& GetTimeline();

/**
 * @brief アクションログ一覧を取得します。
 * @return ログ文字列配列へのconst参照。
 */
const std::vector<std::string>& GetActionLog();

/**
 * @brief 現在のアクターのチームを取得します。
 * @return アクターのチーム番号（0 = enemy, 1 = ally）。
 */
int GetActorTeam();

/**
 * @brief 現在のアクターのスロットインデックスを取得します。
 * @return スロットインデックス（0～4）。
 */
int GetActorSlot();

/**
 * @brief 戦闘開始からの累積時間を取得します。
 * @return 累積時間（float）。
 */
float GetAccumTime();

/**
 * @brief 現在の履歴インデックスを取得します。
 * @return 履歴インデックス。
 */
int GetHistoryIndex();

/**
 * @brief 保存されている履歴総数を取得します。
 * @return 履歴数。
 */
int GetHistoryCount();

/**
 * @brief 現在選択されているスキル種別を取得します。
 * @return 選択中のSkillType。
 */
SkillType GetCurrentSkillType();

/**
 * @brief 現在選択されているスキル種別を切り替えます（BA <-> BS）。
 */
void ToggleCurrentSkillType();

/**
 * @brief タイムラインの初期化および初期アクターの解決を行います。
 */
void InitTimeline();

/**
 * @brief 履歴を1ステップ前に巻き戻します（Undo）。
 */
void BackupTurn();

/**
 * @brief 履歴を1ステップ進めます（Redo）。
 */
void ForwardTurn();

/**
 * @brief 指定したインデックスの履歴ステップへ復元します。
 * @param index 復元する履歴インデックス。
 */
void RestoreHistoryStep(int index);

/**
 * @brief スキル実行およびターゲット選択を伴い、履歴を記録・分岐してターンを進めます。
 * @param skillType 実行するスキル種別。
 * @param targetTeam 対象チーム（0 = enemy, 1 = ally）。
 * @param targetSlot 対象スロットインデックス（0～4）。
 */
void ProceedTurnWithHistory(SkillType skillType, int targetTeam, int targetSlot);

#endif // SIMULATION_LOGIC_HPP
