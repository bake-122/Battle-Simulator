#ifndef EFFECT_LIST_HPP
#define EFFECT_LIST_HPP

#include <cstddef>
#include <cstdint>

#include "../unit_types.h"

// Effectの対象範囲。必要に応じて追加する。
enum class TargetRange : std::uint8_t
{
    Self,
    Single,
    Proximity,
    All
};

// Effect実行時のActor/Target情報。
struct EffectContext
{
    int actorTeam = -1;
    int actorSlot = -1;
    int targetTeam = -1;
    int targetSlot = -1;
    // Effect実行時に空スロットを除外するための占有情報。
    // 配列はExecuteProcess呼び出し中に有効であること。
    bool* allySlotOccupied = nullptr;
    bool* enemySlotOccupied = nullptr;
    // Proximityの隣接対象に対してtrue。
    bool isProximityTarget = false;
    // 対象HPを取得するためのコールバック情報。Effect層はTeamStateに直接依存しない。
    void* hpContext = nullptr;
    int* (*resolveTargetHP)(void*, int, int) = nullptr;
    int* targetHP = nullptr;
    // DealDMG適用直後に死亡判定を行い、占有情報やTimelineを更新する。
    void (*onDamageApplied)(void*, int, int, bool*, bool*) = nullptr;
    // Actor / Target の戦闘ステータス
    int actorBaseAtk = 0;
    float actorTotalAtk = 0.0f;
    float targetTotalDef = 0.0f;
    float (*resolveTargetDef)(void*, int, int) = nullptr;
};

// DealDMG専用の引数データ。
struct DealDMGArgs
{
    Element element = Element::None;
    TargetRange range = TargetRange::Single;
    // dmg / proximity_dmg はパーミル単位（1000 = 100%）。
    int dmg = 0;
    int bgd = 0;
    int proximity_dmg = 0;
    int proximity_bgd = 0;
};

// Effectは連番のIDで管理する。新しいEffectはCountの直前に追加し、
// EffectList.cppの登録表にも同じ順番で実装を登録する。
// uint16_tにより、Countを除いて最大65535個までIDを表現できる。
enum class EffectType : std::uint16_t
{
    DealDMG = 0,
    Count
};

// 引数型の違いはEffectごとの引数構造体で扱い、共通Invokerで呼び出す。
using EffectInvoker = void (*)(const EffectContext&, const void*);

struct EffectDefinition
{
    EffectInvoker invoke = nullptr;
};

// Processが実行するEffectと、そのEffectに渡す引数。
// argumentsは対応するEffectの引数構造体を指す。参照先は呼び出し中有効であること。
struct EffectCall
{
    EffectType type = EffectType::DealDMG;
    const void* arguments = nullptr;
};

// dmg / proximity_dmg はパーミル単位（1000 = 100%）。
// 既存のEffect。実処理は後から実装する。
void DealDMG(Element element, TargetRange range, int dmg, int bgd = 0,
             int proximity_dmg = 0, int proximity_bgd = 0,
             int targetTeam = -1, int targetSlot = -1,
             int* targetHP = nullptr,
             int actorBaseAtk = 0, float actorTotalAtk = 0.0f, float targetTotalDef = 0.0f);
void InvokeDealDMG(const EffectContext& context, const void* arguments);

// EffectTypeに対応する定義を返す。不正なIDならnullptr。
const EffectDefinition* GetEffectDefinition(EffectType type);

// 1つのEffectCallをディスパッチする。不正なIDや未登録Effectは何もしない。
void ExecuteEffect(const EffectCall& effect, const EffectContext& context);

#endif // EFFECT_LIST_HPP
