/**
 * @file EffectList.cpp
 * @brief 戦闘中のEffect実装、引数展開、対象解決およびディスパッチ処理。
 */

#include "EffectList.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <initializer_list>
#include <cmath>
#include <iostream>

namespace
{
    /**
     * @brief 基礎攻撃力から基礎ダメージ量を算出します。
     * @param baseAtk 基礎攻撃力。
     * @return 算出された基礎ダメージ量（float）。
     */
    float getBaseDMG(int baseAtk)
    {
        return static_cast<float>(baseAtk) * ((std::pow(static_cast<float>(baseAtk) / 124.0f, 1.2f) + 12.0f) / 20.0f);
    }

    /**
     * @brief 総攻撃力と総防御力から攻撃倍率（AttackRatio）を算出します。
     * @param totalAtk 総攻撃力。
     * @param totalDef 総防御力。
     * @return 算出された攻撃倍率（float、最大2.0にクランプ）。
     */
    float getAttackRatio(float totalAtk, float totalDef)
    {
        return std::min(((totalAtk + 10.0f) / (totalDef + 10.0f)) * 0.12f, 2.0f);
    }
}

/**
 * @brief HP減算（DealDMG）を実行します。
 * @details ダメージ計算式に基づいてダメージ量を算出し、対象HPを減算します。
 */
void DealDMG(Element element, TargetRange range, int dmg, int bgd,
             int proximity_dmg, int proximity_bgd,
             int targetTeam, int targetSlot, int* targetHP,
             int actorBaseAtk, float actorTotalAtk, float targetTotalDef)
{
    (void)element;
    (void)range;
    (void)proximity_dmg;
    (void)proximity_bgd;
    (void)bgd;
    (void)targetTeam;
    (void)targetSlot;

    if (targetHP != nullptr)
    {
        const float rawDamage = (static_cast<float>(dmg) / 1000.0f) * getBaseDMG(actorBaseAtk) * getAttackRatio(actorTotalAtk, targetTotalDef);
        const int damage = std::max(1, static_cast<int>(std::ceil(rawDamage)));
        *targetHP = std::max(0, *targetHP - damage);
        // Debug Info
        std::cout << "[Damage] " << (targetTeam == 1 ? "Ally" : "Enemy")
              << "[" << targetSlot + 1 << "] takes " << damage
              << " damage (Remaining HP: " << *targetHP << ")" << std::endl;
    }
}

/**
 * @brief DealDMGのInvoker関数。EffectContextから引数を解決してDealDMGを呼び出します。
 */
void InvokeDealDMG(const EffectContext& context, const void* arguments)
{
    if (arguments == nullptr)
    {
        return;
    }

    const auto& args = *static_cast<const DealDMGArgs*>(arguments);
    if (context.isProximityTarget)
    {
        DealDMG(args.element, TargetRange::Single,
                args.proximity_dmg, args.proximity_bgd,
                0, 0, context.targetTeam, context.targetSlot,
                context.targetHP,
                context.actorBaseAtk, context.actorTotalAtk, context.targetTotalDef);
        if (context.targetHP != nullptr && context.onDamageApplied != nullptr)
        {
            context.onDamageApplied(context.hpContext, context.targetTeam,
                                    context.targetSlot, context.allySlotOccupied,
                                    context.enemySlotOccupied);
        }
    }
    else
    {
        DealDMG(args.element, TargetRange::Single, args.dmg, args.bgd,
                0, 0, context.targetTeam, context.targetSlot,
                context.targetHP,
                context.actorBaseAtk, context.actorTotalAtk, context.targetTotalDef);
        if (context.targetHP != nullptr && context.onDamageApplied != nullptr)
        {
            context.onDamageApplied(context.hpContext, context.targetTeam,
                                    context.targetSlot, context.allySlotOccupied,
                                    context.enemySlotOccupied);
        }
    }
}

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
void BuffSPDFixed(int amount, int turns, bool skipSelfEffectTurn, SPDEffectList* target)
{
    if (target == nullptr)
    {
        return;
    }

    SPDEffect effect;
    effect.kind = SPDEffectKind::Fixed;
    effect.amount = static_cast<double>(amount);
    // 効果ターン数が指定されない（-1以下）場合は永続として扱う。
    effect.remainingTurns = (turns > 0) ? turns : -1;
    effect.skipSelfEffectTurn = skipSelfEffectTurn && effect.remainingTurns > 0;

    target->push_back(effect);
}

/**
 * @brief スピードの割合加算（BuffSPDRatio）を実行します。
 * @details 対象の適用中SPD効果リストへ割合加算の効果インスタンスを追加する。
 *          状態変更のみを担当する（BuffSPDFixedと同一）。
 * @param amount 基礎SPDに対する割合（Permill）。
 * @param turns 効果ターン数（-1 = 永続）。
 * @param skipSelfEffectTurn 付与ターンの終了時だけ減算しない（自己付与時）。
 * @param target 対象の適用中SPD効果リスト。
 */
void BuffSPDRatio(int amount, int turns, bool skipSelfEffectTurn, SPDEffectList* target)
{
    if (target == nullptr)
    {
        return;
    }

    SPDEffect effect;
    effect.kind = SPDEffectKind::Ratio;
    effect.amount = static_cast<double>(amount);
    // 効果ターン数が指定されない（-1以下）場合は永続（常時発動型）として扱う。
    effect.remainingTurns = (turns > 0) ? turns : -1;
    effect.skipSelfEffectTurn = skipSelfEffectTurn && effect.remainingTurns > 0;

    target->push_back(effect);
}

/**
 * @brief 整数値SPD加算のInvoker関数。
 * @details 対象の加算固定値型SPD効果合計へ amount を加算し、SPD変化を通知する。
 *          通知は「表示SPDが変わらない再付与」でも発生し、その場合でも
 *          Priorityは発行される。
 *          状態の加算はBuffSPDFixed関数へ委譲し、この関数は引数の解決と
 *          適用後の通知だけを担当する。
 */
void InvokeBuffSPDFixed(const EffectContext& context, const void* arguments)
{
    if (arguments == nullptr)
    {
        return;
    }

    const auto& args = *static_cast<const BuffSPDFixedArgs*>(arguments);

    SPDEffectList* effects = (context.resolveTargetSPDEffects != nullptr)
        ? context.resolveTargetSPDEffects(context.spdContext,
                                          context.targetTeam,
                                          context.targetSlot)
        : nullptr;

    // Self-Granted の場合は付与ターンの終了時だけ減算しない。
    // ただし、Passive 効果では減算免除しない。
    const bool self_granted = (context.actorTeam == context.targetTeam &&
                                context.actorSlot == context.targetSlot);
    const bool skip_self_effect_turn =
        !args.isPassive && self_granted && args.turns > 0;

    BuffSPDFixed(args.amount, args.turns, skip_self_effect_turn, effects);

    // 対象を解決できた場合、AV再計算とPriorityをEffect外のロジックへ通知する。
    if (effects != nullptr && context.onTargetSPDChanged != nullptr)
    {
        context.onTargetSPDChanged(context.spdContext, context.targetTeam,
                                   context.targetSlot, context.turnPriority);
    }
}

/**
 * @brief 割合SPD加算のInvoker関数。
 * @details 対象の基礎SPDへ割合を加算し、SPD変化を通知する。
 *          通知は「表示SPDが変わらない再付与」でも発生し、その場合でも
 *          Priorityは発行される。
 */
void InvokeBuffSPDRatio(const EffectContext& context, const void* arguments)
{
    if (arguments == nullptr)
    {
        return;
    }

    const auto& args = *static_cast<const BuffSPDRatioArgs*>(arguments);

    SPDEffectList* effects = (context.resolveTargetSPDEffects != nullptr)
        ? context.resolveTargetSPDEffects(context.spdContext,
                                          context.targetTeam,
                                          context.targetSlot)
        : nullptr;

    const bool self_granted = (context.actorTeam == context.targetTeam &&
                                context.actorSlot == context.targetSlot);
    const bool skip_self_effect_turn =
        !args.isPassive && self_granted && args.turns > 0;

    BuffSPDRatio(args.amount, args.turns, skip_self_effect_turn, effects);

    if (effects != nullptr && context.onTargetSPDChanged != nullptr)
    {
        context.onTargetSPDChanged(context.spdContext, context.targetTeam,
                                   context.targetSlot, context.turnPriority);
    }
}

namespace battle_effect_detail
{
    // EffectTypeの値を添字として使用する静的登録表。
    // Effectを追加したらEffectTypeとこの表の両方を更新する。
    constexpr std::array<EffectDefinition,
        static_cast<std::size_t>(EffectType::Count)> registry{{
            { &InvokeDealDMG },
            { &InvokeBuffSPDFixed },
            { &InvokeBuffSPDRatio }
        }};

    static_assert(registry.size() == static_cast<std::size_t>(EffectType::Count),
                  "EffectType and Effect registry must have the same size");
}

/**
 * @brief EffectTypeに対応する定義情報を取得します。
 */
const EffectDefinition* GetEffectDefinition(EffectType type)
{
    const std::size_t index = static_cast<std::size_t>(type);
    if (index >= battle_effect_detail::registry.size())
    {
        return nullptr;
    }

    return &battle_effect_detail::registry[index];
}

namespace
{
    constexpr int SLOT_COUNT = 5;

    /**
     * @brief Effect種別に対応する引数構造体から対象範囲を解決します。
     * @details Effectごとに引数構造体の形が異なるため、種別ごとに読み分ける。
     * @param type Effect種別。
     * @param arguments Effectに対応する引数構造体ポインタ。
     * @return 対象範囲。不正な種別や引数nullptrの場合はTargetRange::Single。
     */
    TargetRange ResolveEffectRange(EffectType type, const void* arguments)
    {
        if (arguments == nullptr)
        {
            return TargetRange::Single;
        }

        switch (type)
        {
        case EffectType::DealDMG:
            return static_cast<const DealDMGArgs*>(arguments)->range;
        case EffectType::BuffSPDFixed:
            return static_cast<const BuffSPDFixedArgs*>(arguments)->range;
        case EffectType::BuffSPDRatio:
            return static_cast<const BuffSPDRatioArgs*>(arguments)->range;
        case EffectType::Count:
        default:
            return TargetRange::Single;
        }
    }

    /**
     * @brief 指定したスロットが占有されているか判定します。
     * @param context Effect実行コンテキスト。
     * @param team チーム番号（0 = enemy, 1 = ally）。
     * @param slot スロット番号。
     * @return 占有されていればtrue。
     */
    bool IsSlotOccupied(const EffectContext& context, int team, int slot)
    {
        if (slot < 0 || slot >= SLOT_COUNT)
        {
            return false;
        }

        const bool* occupied = (team == 1)
            ? context.allySlotOccupied
            : context.enemySlotOccupied;

        // 占有情報が渡されない単体テスト等では、範囲内スロットを有効とする。
        return occupied == nullptr || occupied[slot];
    }

    /**
     * @brief 指定したターゲットに対してEffectを呼び出します。
     * @param definition Effect定義。
     * @param effect Effect呼び出し情報。
     * @param baseContext ベースコンテキスト。
     * @param targetTeam 対象チーム番号。
     * @param targetSlot 対象スロット番号。
     * @param isProximityTarget 隣接対象フラグ。
     */
    void InvokeForTarget(const EffectDefinition& definition,
                         const EffectCall& effect,
                         const EffectContext& baseContext,
                         int targetTeam,
                         int targetSlot,
                         bool isProximityTarget = false)
    {
        EffectContext targetContext = baseContext;
        targetContext.targetTeam = targetTeam;
        targetContext.targetSlot = targetSlot;
        targetContext.isProximityTarget = isProximityTarget;
        targetContext.targetHP =
            (targetContext.resolveTargetHP != nullptr)
                ? targetContext.resolveTargetHP(
                    targetContext.hpContext, targetTeam, targetSlot)
                : nullptr;
        targetContext.targetTotalDef =
            (targetContext.resolveTargetDef != nullptr)
                ? targetContext.resolveTargetDef(
                    targetContext.hpContext, targetTeam, targetSlot)
                : 0.0f;
        definition.invoke(targetContext, effect.arguments);
    }
}

/**
 * @brief 1つのEffectCallを実行・ディスパッチします。
 */
void ExecuteEffect(const EffectCall& effect, const EffectContext& context)
{
    const EffectDefinition* definition = GetEffectDefinition(effect.type);
    if (definition == nullptr || definition->invoke == nullptr ||
        effect.arguments == nullptr)
    {
        return;
    }

    // Process実行中にActorが死亡した場合、以降のEffectを発動させない。
    if (!IsSlotOccupied(context, context.actorTeam, context.actorSlot))
    {
        return;
    }

    // 対象範囲はEffect種別ごとの引数構造体から解決する。
    switch (ResolveEffectRange(effect.type, effect.arguments))
    {
    case TargetRange::Self:
        if (IsSlotOccupied(context, context.actorTeam, context.actorSlot))
        {
            InvokeForTarget(*definition, effect, context,
                            context.actorTeam, context.actorSlot);
        }
        break;

    case TargetRange::Single:
        if (IsSlotOccupied(context, context.targetTeam, context.targetSlot))
        {
            InvokeForTarget(*definition, effect, context,
                            context.targetTeam, context.targetSlot);
        }
        break;

    case TargetRange::Proximity:
    {
        if (!IsSlotOccupied(context, context.targetTeam, context.targetSlot))
        {
            break;
        }

        // 主対象に命中した後、画面上の左から右の順に隣接スロットを確認する。
        InvokeForTarget(*definition, effect, context,
                        context.targetTeam, context.targetSlot);
        const int leftAdjacent = (context.targetTeam == 0)
            ? context.targetSlot + 1 : context.targetSlot - 1;
        const int rightAdjacent = (context.targetTeam == 0)
            ? context.targetSlot - 1 : context.targetSlot + 1;
        for (const int adjacentSlot : { leftAdjacent, rightAdjacent })
        {
            if (IsSlotOccupied(context, context.targetTeam, adjacentSlot))
            {
                InvokeForTarget(*definition, effect, context,
                                context.targetTeam, adjacentSlot, true);
            }
        }
        break;
    }

    case TargetRange::All:
    {
        // AllはActorの対面チーム全体をUIから見て左から右の順で対象とする。
        // 敵スロットは表示時に左右反転するため、チームに応じて走査方向を変える。
        // 各対象へのInvokeForTarget内で死亡処理まで完了するので、次の対象は更新後の占有情報で判定される。
        const int opposingTeam = (context.actorTeam == 1) ? 0 : 1;
        const int firstSlot = (opposingTeam == 0) ? SLOT_COUNT - 1 : 0;
        const int endSlot = (opposingTeam == 0) ? -1 : SLOT_COUNT;
        const int step = (opposingTeam == 0) ? -1 : 1;
        for (int slot = firstSlot; slot != endSlot; slot += step)
        {
            if (IsSlotOccupied(context, opposingTeam, slot))
            {
                InvokeForTarget(*definition, effect, context, opposingTeam, slot);
            }
        }
        break;
    }
    }
}
