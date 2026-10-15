#include "EffectList.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <initializer_list>
#include <cmath>

#include <iostream>

namespace
{
    float getBaseDMG(int baseAtk)
    {
        return static_cast<float>(baseAtk) * ((std::pow(static_cast<float>(baseAtk) / 124.0f, 1.2f) + 12.0f) / 20.0f);
    }

    float getAttackRatio(float totalAtk, float totalDef)
    {
        return std::min(((totalAtk + 10.0f) / (totalDef + 10.0f)) * 0.12f, 2.0f);
    }
}

// HPを減算する。dmgは1/1000
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

namespace exedra_effect_detail
{
    // EffectTypeの値を添字として使用する静的登録表。
    // Effectを追加したらEffectTypeとこの表の両方を更新する。
    constexpr std::array<EffectDefinition,
        static_cast<std::size_t>(EffectType::Count)> registry{{
            { &InvokeDealDMG }
        }};

    static_assert(registry.size() == static_cast<std::size_t>(EffectType::Count),
                  "EffectType and Effect registry must have the same size");
}

const EffectDefinition* GetEffectDefinition(EffectType type)
{
    const std::size_t index = static_cast<std::size_t>(type);
    if (index >= exedra_effect_detail::registry.size())
    {
        return nullptr;
    }

    return &exedra_effect_detail::registry[index];
}

namespace
{
    constexpr int SLOT_COUNT = 5;

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

    const auto& args = *static_cast<const DealDMGArgs*>(effect.arguments);

    switch (args.range)
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
