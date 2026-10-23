/**
 * @file EffectList.hpp
 * @brief 戦闘中のEffect定義、引数構造体、ディスパッチ処理を提供するヘッダーファイル。
 */

#ifndef EFFECT_LIST_HPP
#define EFFECT_LIST_HPP

#include <cstddef>
#include <cstdint>
#include <vector>

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

// Temporary

/**
 * @brief SPD効果の種別。
 * @details Fixed, RatioはbaseSPDに対する加算的効果として扱う
 *          （乗算的効果・2パス合成は未実装）。
 */
enum class SPDEffectKind : std::uint8_t
{
    Fixed = 0, /**< BuffSPDFixed */
    Ratio = 1  /**< BuffSPDRatio(Permill) */
};

/**
 * @brief 1件のSPD効果（適用インスタンス）。
 * @details 状態モデルの一つ。
 *          残りターン数が -1 のものは常時発動型（永続）として扱う。
 */
struct SPDEffect
{
    SPDEffectKind kind = SPDEffectKind::Fixed; /**< 効果種別 */
    int amount = 0;      /**< Fixed: 固定値 / Ratio: 基礎SPDに対するPermill */
    int remainingTurns = -1;  /**< 残りターン数（-1 = 永続） */
    bool skipSelfEffectTurn = false; /**< 付与ターンの終了時だけ減算しない（自己付与時） */
    std::uint32_t sourceProcessId = 0; /**< 付与元のProcess ID（再付与判定用・未使用） */
};

/**
 * @brief 適用中のSPD効果のリスト。
 */
using SPDEffectList = std::vector<SPDEffect>;

/**
 * @brief Effect実行時のコンテキスト情報（Actor/Targetおよびコールバック）。
 */
struct EffectContext
{
    int actorTeam = -1;                                              /**< Actorのチーム番号（0 = enemy, 1 = ally） */
    int actorSlot = -1;                                              /**< Actorのスロットインデックス（0～4） */
    int targetTeam = -1;                                             /**< ターゲットのチーム番号（0 = enemy, 1 = ally） */
    int targetSlot = -1;                                             /**< ターゲットのスロットインデックス（0～4） */

    bool* allySlotOccupied = nullptr;                                /**< 味方スロットの占有状態配列ポインタ */
    bool* enemySlotOccupied = nullptr;                               /**< 敵スロットの占有状態配列ポインタ */
    bool isProximityTarget = false;                                  /**< Proximityの隣接対象であるかどうかのフラグ */

    void* hpContext = nullptr;                                       /**< HP解決コールバック用のコンテキストポインタ */
    int* (*resolveTargetHP)(void*, int, int) = nullptr;              /**< 対象スロットの現在HPポインタを解決する関数ポインタ */
    int* targetHP = nullptr;                                         /**< 解決された対象HPへのポインタ */

    void (*onDamageApplied)(void*, int, int, bool*, bool*) = nullptr;/**< ダメージ適用直後に死亡判定・占有更新等を行うコールバック */

    int actorBaseAtk = 0;                                            /**< Actorの基礎攻撃力 */
    float actorTotalAtk = 0.0f;                                      /**< Actorの総攻撃力 */
    float targetTotalDef = 0.0f;                                     /**< ターゲットの総防御力 */
    float (*resolveTargetDef)(void*, int, int) = nullptr;            /**< 対象スロットの総防御力を解決する関数ポインタ */

    // ------------------------------------------------------------
    // SPD 効果用
    // ------------------------------------------------------------

    void* spdContext = nullptr;                                      /**< SPD解決コールバック用のコンテキストポインタ */
    SPDEffectList* (*resolveTargetSPDEffects)(void*, int, int) = nullptr; /**< 対象スロットの適用中SPD効果リストへのポインタを解決する関数ポインタ */
    void (*onTargetSPDChanged)(void*, int, int, std::uint32_t) = nullptr; /**< 対象SPD変化時のAV再計算・Priority更新を行うコールバック */
    std::uint32_t turnPriority = 0;                                 /**< 発行済みのPriority値（対象へ更新） */
};

/**
 * @brief DealDMG専用の引数データ。
 */
struct DealDMGArgs
{
    Element element = Element::None;  /**< 攻撃属性 */
    TargetRange range = TargetRange::Single; /**< 対象範囲 */
    int dmg = 0;                      /**< メインターゲットへのダメージ倍率（Permill） */
    int bgd = 0;                      /**< メインターゲットへのブレイクゲージ削減量 */
    int proximity_dmg = 0;            /**< 隣接ターゲットへのダメージ倍率（Permill） */
    int proximity_bgd = 0;            /**< 隣接ターゲットへのブレイクゲージ削減量 */
};

/**
 * @brief BuffSPDFixed（スピード固定値加算効果）用の引数データ。
 * @details 固定値加算型のSPD効果。対象の加算型SPD効果合計へ
 *          amount を加算する。
 */
struct BuffSPDFixedArgs
{
    TargetRange range = TargetRange::Self; /**< 対象範囲 */
    double amount = 0.0;                   /**< 加算するSPD値（固定値） */
    int turns = -1;                        /**< 効果ターン数（-1 = 永続） */
    bool isPassive = false;                /**< Passive効果か（=バフ解除の対象か） */
};

/**
 * @brief BuffSPDRatio（スピード割合加算効果）用の引数データ。
 * @details 割合加算型のSPD効果。対象の基礎SPDへ
 *          amount（Permill）を乗じた値を加算する。
 */
struct BuffSPDRatioArgs
{
    TargetRange range = TargetRange::Self; /**< 対象範囲 */
    int amount = 0;                        /**< 基礎SPDに対する割合（Permill） */
    int turns = -1;                        /**< 効果ターン数（-1 = 永続） */
    bool isPassive = false;                /**< Passive効果か（=バフ解除の対象か） */
};

/**
 * @brief Effectの識別種別。
 * @details 連番のIDで管理され、Countを除いて最大65535個までIDを表現可能。
 */
enum class EffectType : std::uint16_t
{
    DealDMG = 0,
    BuffSPDFixed,
    BuffSPDRatio,
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
 * @param bgd ブレイクゲージ削減量。
 * @param proximity_dmg 隣接へのダメージ倍率（パーミル）。
 * @param proximity_bgd 隣接へのブレイクゲージ削減量。
 * @param targetTeam 対象チーム番号。
 * @param targetSlot 対象スロット番号。
 * @param targetHP 対象HPポインタ。
 * @param actorBaseAtk Actorの基礎攻撃力。
 * @param actorTotalAtk Actorの総攻撃力。
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
 * @brief スピードの固定値加算（BuffSPDFixed）を実行します。
 * @details 対象の適用中SPD効果リストへ固定値加算の効果インスタンスを追加する。
 *          状態変更のみを担当し、AV再計算・Priority更新などの適用後処理は
 *          Invoker側で行う。
 * @param amount 加算するSPD値（固定値）。
 * @param turns 効果ターン数（-1 = 永続）。
 * @param skipSelfEffectTurn 付与ターンの終了時だけ減算しない（自己付与時）。
 * @param target 対象の適用中SPD効果リスト。
 */
void BuffSPDFixed(int amount, int turns, bool skipSelfEffectTurn, SPDEffectList* target);

/**
 * @brief スピードの割合加算（BuffSPDRatio）を実行します。
 * @details 対象の適用中SPD効果リストへ割合加算の効果インスタンスを追加する。
 *          状態変更のみを担当する（BuffSPDFixedと同一）。
 * @param amount 基礎SPDに対する割合（Permill）。
 * @param turns 効果ターン数（-1 = 永続）。
 * @param skipSelfEffectTurn 付与ターンの終了時だけ減算しない（自己付与時）。
 * @param target 対象の適用中SPD効果リスト。
 */
void BuffSPDRatio(int amount, int turns, bool skipSelfEffectTurn, SPDEffectList* target);

/**
 * @brief BuffSPDFixed のInvoker関数。
 * @details EffectContextから引数を解決して、対象の加算固定値型SPDの合計へ加算する。
 * @param context Effect実行コンテキスト。
 * @param arguments BuffSPDFixedArgsポインタ。
 */
void InvokeBuffSPDFixed(const EffectContext& context, const void* arguments);

/**
 * @brief BuffSPDRatio のInvoker関数。
 * @details EffectContextから引数を解決して、対象の基礎SPDへ割合を加算する。
 * @param context Effect実行コンテキスト。
 * @param arguments BuffSPDRatioArgsポインタ。
 */
void InvokeBuffSPDRatio(const EffectContext& context, const void* arguments);

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
