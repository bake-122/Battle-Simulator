/**
 * @file EffectList.hpp
 * @brief 戦闘中のEffect定義、引数構造体、ディスパッチ処理を提供するヘッダーファイル。
 */

#ifndef EFFECT_LIST_HPP
#define EFFECT_LIST_HPP

#include <cstddef>
#include <cstdint>

#include "../unit_types.h"

/**
 * @brief Effectの対象範囲。
 */
enum class TargetRange : std::uint8_t
{
    Self,       /**< 自身のみ */
    Single,     /**< 単体 */
    Proximity,  /**< 単体および左右隣接スロット */
    All         /**< 全体 */
};

/**
 * @brief Effect実行時のコンテキスト情報（Actor/Targetおよびコールバック）。
 */
struct EffectContext
{
    int actorTeam = -1;                                              /**< アクターのチーム番号（0 = enemy, 1 = ally） */
    int actorSlot = -1;                                              /**< アクターのスロットインデックス（0～4） */
    int targetTeam = -1;                                             /**< ターゲットのチーム番号（0 = enemy, 1 = ally） */
    int targetSlot = -1;                                             /**< ターゲットのスロットインデックス（0～4） */

    bool* allySlotOccupied = nullptr;                                /**< 味方スロットの占有状態配列ポインタ */
    bool* enemySlotOccupied = nullptr;                               /**< 敵スロットの占有状態配列ポインタ */
    bool isProximityTarget = false;                                  /**< Proximityの隣接対象であるかどうかのフラグ */

    void* hpContext = nullptr;                                       /**< HP解決コールバック用のコンテキストポインタ */
    int* (*resolveTargetHP)(void*, int, int) = nullptr;              /**< 対象スロットの現在HPポインタを解決する関数ポインタ */
    int* targetHP = nullptr;                                         /**< 解決された対象HPへのポインタ */

    void (*onDamageApplied)(void*, int, int, bool*, bool*) = nullptr;/**< ダメージ適用直後に死亡判定・占有更新等を行うコールバック */

    int actorBaseAtk = 0;                                            /**< アクターの基礎攻撃力 */
    float actorTotalAtk = 0.0f;                                      /**< アクターの総攻撃力 */
    float targetTotalDef = 0.0f;                                     /**< ターゲットの総防御力 */
    float (*resolveTargetDef)(void*, int, int) = nullptr;            /**< 対象スロットの総防御力を解決する関数ポインタ */
};

/**
 * @brief DealDMG専用の引数データ。
 */
struct DealDMGArgs
{
    Element element = Element::None;  /**< 攻撃属性 */
    TargetRange range = TargetRange::Single; /**< 対象範囲 */
    int dmg = 0;                      /**< メインターゲットへのダメージ倍率（パーミル単位、1000 = 100%） */
    int bgd = 0;                      /**< メインターゲットへのBreak Gauge Damage */
    int proximity_dmg = 0;            /**< 隣接ターゲットへのダメージ倍率（パーミル単位、1000 = 100%） */
    int proximity_bgd = 0;            /**< 隣接ターゲットへのBreak Gauge Damage */
};

/**
 * @brief Effectの識別種別。
 * @details 連番のIDで管理され、Countを除いて最大65535個までIDを表現可能。
 */
enum class EffectType : std::uint16_t
{
    DealDMG = 0, /**< ダメージ処理 */
    Count        /**< Effect総数 */
};

/**
 * @brief 共通Effect実行関数シグネチャ。
 */
using EffectInvoker = void (*)(const EffectContext&, const void*);

/**
 * @brief Effectの定義情報構造体。
 */
struct EffectDefinition
{
    EffectInvoker invoke = nullptr; /**< Effect実行関数ポインタ */
};

/**
 * @brief Processが実行するEffectと引数のペア。
 */
struct EffectCall
{
    EffectType type = EffectType::DealDMG; /**< 呼び出すEffect種別 */
    const void* arguments = nullptr;        /**< Effectに対応する引数構造体ポインタ */
};

/**
 * @brief ダメージ処理（DealDMG）を実行します。
 * @param element 攻撃属性。
 * @param range 対象範囲。
 * @param dmg ダメージ倍率（パーミル）。
 * @param bgd ブレイクゲージダメージ。
 * @param proximity_dmg 隣接対象ダメージ倍率（パーミル）。
 * @param proximity_bgd 隣接対象ブレイクゲージダメージ。
 * @param targetTeam 対象チーム番号。
 * @param targetSlot 対象スロット番号。
 * @param targetHP 対象HPポインタ。
 * @param actorBaseAtk アクター基礎攻撃力。
 * @param actorTotalAtk アクター総攻撃力。
 * @param targetTotalDef ターゲット総防御力。
 */
void DealDMG(Element element, TargetRange range, int dmg, int bgd = 0,
             int proximity_dmg = 0, int proximity_bgd = 0,
             int targetTeam = -1, int targetSlot = -1,
             int* targetHP = nullptr,
             int actorBaseAtk = 0, float actorTotalAtk = 0.0f, float targetTotalDef = 0.0f);

/**
 * @brief DealDMGのInvoker関数。EffectContextから引数を解決してDealDMGを呼び出します。
 * @param context Effect実行コンテキスト。
 * @param arguments DealDMGArgsポインタ。
 */
void InvokeDealDMG(const EffectContext& context, const void* arguments);

/**
 * @brief EffectTypeに対応する定義情報を取得します。
 * @param type Effect種別。
 * @return 定義情報へのポインタ。不正なIDならnullptr。
 */
const EffectDefinition* GetEffectDefinition(EffectType type);

/**
 * @brief 1つのEffectCallを実行・ディスパッチします。
 * @param effect 実行するEffect情報。
 * @param context Effect実行コンテキスト。
 */
void ExecuteEffect(const EffectCall& effect, const EffectContext& context);

#endif // EFFECT_LIST_HPP
