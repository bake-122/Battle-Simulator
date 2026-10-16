/**
 * @file simulationLogic.cpp
 * @brief 戦闘シミュレーションのロジック・タイムライン進行・履歴管理の実装。
 */

#include "simulationLogic.hpp"

#include <algorithm>
#include <array>
#include <cstdio>
#include <cmath>
#include <utility>

namespace
{
    TeamState g_teamState{};
    SkillType g_currentSkillType = SkillType::BA;

    void HandleDamageApplied(void* context, int team, int slot,
                             bool* allySlotOccupied, bool* enemySlotOccupied);

    /**
     * @brief 対象キオクの現在HPポインタを解決します。
     * @param context チーム状態へのポインタ（TeamState*）。
     * @param team チーム番号（0 = enemy, 1 = ally）。
     * @param slot スロット番号（0～4）。
     * @return 対象HPへのポインタ。不正時はnullptr。
     */
    int* ResolveTargetHP(void* context, int team, int slot)
    {
        if (context == nullptr || slot < 0 || slot >= 5 ||
            (team != 0 && team != 1))
        {
            return nullptr;
        }

        auto* state = static_cast<TeamState*>(context);
        return (team == 1)
            ? &state->ally[slot].hp
            : &state->enemy[slot].hp;
    }

    /**
     * @brief 対象キオクの総防御力を解決します。
     * @param context チーム状態へのポインタ（TeamState*）。
     * @param team チーム番号（0 = enemy, 1 = ally）。
     * @param slot スロット番号（0～4）。
     * @return 総防御力（float）。不正時は0.0f。
     */
    float ResolveTargetDef(void* context, int team, int slot)
    {
        if (context == nullptr || slot < 0 || slot >= 5 ||
            (team != 0 && team != 1))
        {
            return 0.0f;
        }

        auto* state = static_cast<TeamState*>(context);
        const auto& unit = (team == 1)
            ? state->ally[slot]
            : state->enemy[slot];
        return static_cast<float>(unit.def);
    }
}

namespace
{
    /**
     * @brief 履歴スナップショット構造体。
     */
    struct HistorySnapshot
    {
        TeamState teamState;                       /**< チーム状態 */
        std::vector<TimelineNode> timeline;        /**< タイムライン配列 */
        int actorTeam = -1;                        /**< アクターのチーム */
        int actorSlot = -1;                        /**< アクターのスロット */
        int selectedTargetTeam = -1;               /**< 選択されたターゲットチーム */
        int selectedTargetSlot = -1;               /**< 選択されたターゲットスロット */
        float accumTime = 0.0f;                    /**< 累積時間 */
    };
}

// ===========================================================
// タイムライン・行動ログ
// ===========================================================

static std::vector<TimelineNode> g_timeline;
static std::vector<std::string> g_actionLog;

// 現在の Actor (タイムライン先頭)
static int g_actorTeam = -1;     // 0=enemy, 1=ally, -1=none
static int g_actorSlot = -1;

// 選択中のターゲット
static int g_selectedTargetTeam = -1;
static int g_selectedTargetSlot = -1;

// 累積時刻 t (minAV 減算の累積値)
static float g_accumTime = 0.0f;

// 履歴管理
static std::vector<HistorySnapshot> g_historySequence;

/**
 * @brief 各履歴ステップで実行された行動情報。
 */
struct HistoryAction
{
    SkillType skillType = SkillType::BA; /**< スキル種別 */
    int targetTeam = -1;                 /**< ターゲットチーム */
    int targetSlot = -1;                 /**< ターゲットスロット */
};

static std::vector<HistoryAction> g_historyActions;

// 現在表示している状態のインデックス。
// 0 = 初期状態
static int g_historyIndex = 0;

namespace
{
    /**
     * @brief 付与効果・被付与効果の解除処理（将来実装用のスタブ）。
     * @param team チーム番号。
     * @param slot スロット番号。
     */
    void RemoveEffectsGivenOrReceived(int team, int slot)
    {
        (void)team;
        (void)slot;
    }

    /**
     * @brief DealDMG直後に死亡判定・占有更新・タイムライン更新を行うコールバック。
     * @param context チーム状態ポインタ。
     * @param team 対象チーム番号。
     * @param slot 対象スロット番号。
     * @param allySlotOccupied 味方スロット占有配列。
     * @param enemySlotOccupied 敵スロット占有配列。
     */
    void HandleDamageApplied(void* context, int team, int slot,
                             bool* allySlotOccupied, bool* enemySlotOccupied)
    {
        if (context == nullptr || slot < 0 || slot >= 5 ||
            (team != 0 && team != 1))
        {
            return;
        }

        auto* state = static_cast<TeamState*>(context);
        UnitStatDetails& target = (team == 1)
            ? state->ally[slot]
            : state->enemy[slot];

        if (target.character.id == 0 || target.hp > 0)
        {
            return;
        }

        // 死亡したキオクの個人ゲージを消去する。SPはパーティ共有値なので変更しない。
        target.currentMP = 0;
        target.currentBP = 0;
        RemoveEffectsGivenOrReceived(team, slot);

        if (team == 1 && allySlotOccupied != nullptr)
        {
            allySlotOccupied[slot] = false;
        }
        else if (team == 0 && enemySlotOccupied != nullptr)
        {
            enemySlotOccupied[slot] = false;
        }

        g_timeline.erase(
            std::remove_if(g_timeline.begin(), g_timeline.end(),
                [team, slot](const TimelineNode& node)
                {
                    return node.team == team && node.slotIndex == slot;
                }),
            g_timeline.end()
        );

        // Actorが死亡した場合も、Actor参照を死亡キオクに残さない。
        if (g_actorTeam == team && g_actorSlot == slot)
        {
            if (g_timeline.empty())
            {
                g_actorTeam = -1;
                g_actorSlot = -1;
            }
            else
            {
                g_actorTeam = g_timeline.front().team;
                g_actorSlot = g_timeline.front().slotIndex;
            }
        }
    }
}

/**
 * @brief ターンを1回分進行させ、スキル実行・AV更新・ログ記録を行います。
 * @param skillType 実行するスキル種別。
 * @param actorTeam アクターのチーム番号。
 * @param actorSlot アクターのスロットインデックス。
 * @param targetTeam ターゲットのチーム番号。
 * @param targetSlot ターゲットのスロットインデックス。
 */
static void ProceedTurn(SkillType skillType, int actorTeam, int actorSlot, int targetTeam, int targetSlot);

/**
 * @brief 現在のチーム状態への参照を取得します。
 */
const TeamState& GetSimulationTeamState()
{
    return g_teamState;
}

/**
 * @brief ロジック側のチーム状態を設定・初期化します。
 */
void SetSimulationTeamStateLogic(const TeamState& state)
{
    g_teamState = state;

    // 戦闘開始時のHPを最大HPとして固定する。
    for (int i = 0; i < 5; ++i)
    {
        for (UnitStatDetails* slot : { &g_teamState.ally[i], &g_teamState.enemy[i] })
        {
            if (slot->character.id != 0)
            {
                slot->hp = std::max(0, slot->hp);
                // 戦闘開始時点のHPを最大HPとして固定する。
                slot->maxHP = slot->hp;
                if (slot->hp == 0)
                {
                    slot->currentMP = 0;
                    slot->currentBP = 0;
                }
            }
            else
            {
                slot->hp = 0;
                slot->maxHP = 0;
            }
        }
    }

    InitTimeline();
}

/**
 * @brief 現在のタイムラインを取得します。
 */
const std::vector<TimelineNode>& GetTimeline()
{
    return g_timeline;
}

/**
 * @brief アクションログ一覧を取得します。
 */
const std::vector<std::string>& GetActionLog()
{
    return g_actionLog;
}

/**
 * @brief 現在のアクターのチームを取得します。
 */
int GetActorTeam() { return g_actorTeam; }

/**
 * @brief 現在のアクターのスロットインデックスを取得します。
 */
int GetActorSlot() { return g_actorSlot; }

/**
 * @brief 戦闘開始からの累積時間を取得します。
 */
float GetAccumTime() { return g_accumTime; }

/**
 * @brief 現在の履歴インデックスを取得します。
 */
int GetHistoryIndex() { return g_historyIndex; }

/**
 * @brief 保存されている履歴総数を取得します。
 */
int GetHistoryCount() { return static_cast<int>(g_historySequence.size()); }

/**
 * @brief 現在選択されているスキル種別を取得します。
 */
SkillType GetCurrentSkillType() { return g_currentSkillType; }

/**
 * @brief 現在選択されているスキル種別を切り替えます（BA <-> BS）。
 */
void ToggleCurrentSkillType()
{
    // SA は常時選択不可。BA と BS の間だけを切り替える。
    g_currentSkillType = (g_currentSkillType == SkillType::BA)
        ? SkillType::BS
        : SkillType::BA;
}

/**
 * @brief タイムラインをソートします（AV昇順 -> TP昇順）。
 */
static void SortTimeline()
{
    std::sort(g_timeline.begin(), g_timeline.end(),
        [](const TimelineNode& a, const TimelineNode& b)
        {
            if (a.av != b.av) return a.av < b.av;
            return a.tp < b.tp;
        }
    );
}

/**
 * @brief 現在の状態をスナップショットとして履歴に保存します。
 */
static void CaptureSnapshot()
{
    HistorySnapshot snap;
    snap.teamState = g_teamState;
    snap.timeline = g_timeline;
    snap.actorTeam = g_actorTeam;
    snap.actorSlot = g_actorSlot;
    snap.selectedTargetTeam = g_selectedTargetTeam;
    snap.selectedTargetSlot = g_selectedTargetSlot;
    snap.accumTime = g_accumTime;
    g_historySequence.push_back(std::move(snap));
}

/**
 * @brief 指定されたインデックスのシミュレーション状態を正確に復元します。
 */
void RestoreHistoryStep(int index)
{
    if (index < 0 ||
        index >= static_cast<int>(g_historySequence.size()))
    {
        return;
    }

    const HistorySnapshot& snap = g_historySequence[index];

    // チーム状態とタイムラインを保存時点へ戻す
    g_teamState = snap.teamState;
    g_timeline = snap.timeline;

    // Actor を復元
    g_actorTeam = snap.actorTeam;
    g_actorSlot = snap.actorSlot;

    // ターゲット選択状態を復元
    g_selectedTargetTeam = snap.selectedTargetTeam;
    g_selectedTargetSlot = snap.selectedTargetSlot;

    // 累積時刻を復元
    g_accumTime = snap.accumTime;

    // 現在表示している状態のインデックス
    g_historyIndex = index;
}

/**
 * @brief タイムラインの初期化および初期アクターの解決を行います。
 */
void InitTimeline()
{
    g_timeline.clear();
    g_actionLog.clear();
    g_actorTeam = -1;
    g_actorSlot = -1;
    g_selectedTargetTeam = -1;
    g_selectedTargetSlot = -1;
	g_accumTime = 0.0f;

    // 敵チーム (team=0, tp=1~5)
    for (int i = 0; i < 5; ++i)
    {
        const UnitStatDetails& slot = g_teamState.enemy[i];
        if (slot.character.id != 0 && slot.hp > 0)
        {
            TimelineNode node;
            node.team = 0;
            node.slotIndex = i;
            node.tp = i + 1;
            node.id = slot.character.id;
            node.name = slot.character.name;
            node.spd = slot.character.spd;
            node.av = 10000.0f / static_cast<float>(node.spd);
            g_timeline.push_back(node);
        }
    }

    // 味方チーム (team=1, tp=6~10)
    for (int i = 0; i < 5; ++i)
    {
        const UnitStatDetails& slot = g_teamState.ally[i];
        if (slot.character.id != 0 && slot.hp > 0)
        {
            TimelineNode node;
            node.team = 1;
            node.slotIndex = i;
            node.tp = i + 6;
            node.id = slot.character.id;
            node.name = slot.character.name;
            node.spd = slot.character.spd;
            node.av = 10000.0f / static_cast<float>(node.spd);
            g_timeline.push_back(node);
        }
    }

    // 初期ソート
    SortTimeline();

    // min AV を減算して先頭の Actor を決定
    if (!g_timeline.empty())
    {
        const float minAV = g_timeline[0].av;
        for (auto& node : g_timeline)
        {
            node.av -= minAV;
        }
        g_actorTeam = g_timeline[0].team;
        g_actorSlot = g_timeline[0].slotIndex;
		g_accumTime += minAV;
    }
    // 履歴を初期化 — 初期状態のスナップショットを保存
    g_historySequence.clear();
	g_historyActions.clear();
    g_historyIndex = 0;
    CaptureSnapshot();
}

/**
 * @brief 履歴を1ステップ前に巻き戻します（Undo）。
 */
void BackupTurn()
{
    if (g_historyIndex <= 0)
    {
        return;
    }

    RestoreHistoryStep(g_historyIndex - 1);
}

/**
 * @brief 履歴を1ステップ進めます（Redo）。
 */
void ForwardTurn()
{
    const int lastIndex =
        static_cast<int>(g_historySequence.size()) - 1;

    if (g_historyIndex >= lastIndex)
    {
        return;
    }

    RestoreHistoryStep(g_historyIndex + 1);
}

/**
 * @brief スキル実行およびターゲット選択を伴い、履歴を記録・分岐してターンを進めます。
 */
void ProceedTurnWithHistory(SkillType skillType, int targetTeam, int targetSlot)
{
    // 現段階で実行できるSkillTypeはBAとBS。
    if (skillType != SkillType::BA && skillType != SkillType::BS) return;

    // Actor が未設定なら進行不可
    if (g_actorTeam < 0 || g_actorSlot < 0)
    {
        return;
    }
    if (g_actorSlot >= 5 || (g_actorTeam != 0 && g_actorTeam != 1))
    {
        return;
    }
    const auto& currentActor = (g_actorTeam == 1)
        ? g_teamState.ally[g_actorSlot]
        : g_teamState.enemy[g_actorSlot];
    if (currentActor.character.id == 0 || currentActor.hp <= 0)
    {
        return;
    }

    // -------------------------------------------------------
    // 次の行動が履歴に存在し、ターゲットも一致する場合
    // 保存済みの状態を再生する
    // -------------------------------------------------------

    if (g_historyIndex < static_cast<int>(g_historyActions.size()))
    {
        const HistoryAction& nextAction =
            g_historyActions[g_historyIndex];

        if (nextAction.skillType == skillType &&
            nextAction.targetTeam == targetTeam &&
            nextAction.targetSlot == targetSlot)
        {
            RestoreHistoryStep(g_historyIndex + 1);
            return;
        }
    }

    // -------------------------------------------------------
    // 過去の状態から別のターゲットを選んだ場合
    // その時点より未来の履歴を破棄する
    // -------------------------------------------------------

    if (g_historyIndex + 1 <
        static_cast<int>(g_historySequence.size()))
    {
        // 現在の状態まで残す
        g_historySequence.resize(g_historyIndex + 1);

        // 現在の状態までに実行した行動だけ残す
        g_historyActions.resize(g_historyIndex);
        g_actionLog.resize(g_historyIndex);
    }

    // -------------------------------------------------------
    // 新しい行動を実行する
    // -------------------------------------------------------

    g_selectedTargetTeam = targetTeam;
    g_selectedTargetSlot = targetSlot;

    ProceedTurn(
        skillType,
        g_actorTeam,
        g_actorSlot,
        targetTeam,
        targetSlot
    );

    // この行動で選択されたターゲットを保存
    g_historyActions.push_back({
        skillType,
        targetTeam,
        targetSlot
    });

    // ProceedTurn() 実行後の状態を保存
    CaptureSnapshot();

    // 最新の状態へ移動
    g_historyIndex =
        static_cast<int>(g_historySequence.size()) - 1;
}

/**
 * @brief ターンを1回分進行させ、スキル実行・AV更新・ログ記録を行います。
 */
static void ProceedTurn(SkillType skillType, int actorTeam, int actorSlot, int targetTeam, int targetSlot)
{
    if (g_timeline.empty()) return;

    // Actor の名前と Target の名前を取得
    const char* actorName = nullptr;
    int actorSpd = 1;
    for (const auto& node : g_timeline)
    {
        if (node.team == actorTeam && node.slotIndex == actorSlot)
        {
            actorName = node.name;
            actorSpd = node.spd;
            break;
        }
    }

    const char* targetName = nullptr;
    if (targetTeam == 0)
    {
        if (targetSlot >= 0 && targetSlot < 5 && g_teamState.enemy[targetSlot].character.id != 0)
            targetName = g_teamState.enemy[targetSlot].character.name;
    }
    else
    {
        if (targetSlot >= 0 && targetSlot < 5 && g_teamState.ally[targetSlot].character.id != 0)
            targetName = g_teamState.ally[targetSlot].character.name;
    }

    if (actorName == nullptr) actorName = "Unknown";
    if (targetName == nullptr) targetName = "Unknown";

    // BA/BS実行前処理：SPを+1（上限6）、ActorのMPを+15。
    UnitStatDetails& actor = (actorTeam == 1)
        ? g_teamState.ally[actorSlot]
        : g_teamState.enemy[actorSlot];
    g_teamState.sp = std::min(g_teamState.sp + 1, 6);
    actor.currentMP += 15;

    // ActorのキオクID・ML・BAレベルからProcess IDを生成。
    // SkillTypeに対応する共有デバッグProcessを実行。
    // BA/BS/SAのProcess定義はそれぞれ1つずつ登録され、スロットごとに複製しない。
    bool allySlotOccupied[5]{};
    bool enemySlotOccupied[5]{};
    for (int i = 0; i < 5; ++i)
    {
        allySlotOccupied[i] = (g_teamState.ally[i].character.id != 0 &&
                               g_teamState.ally[i].hp > 0);
        enemySlotOccupied[i] = (g_teamState.enemy[i].character.id != 0 &&
                                g_teamState.enemy[i].hp > 0);
    }

    EffectContext effectContext{
        actorTeam,
        actorSlot,
        targetTeam,
        targetSlot,
        allySlotOccupied,
        enemySlotOccupied,
        false
    };
    effectContext.hpContext = &g_teamState;
    effectContext.resolveTargetHP = &ResolveTargetHP;
    effectContext.onDamageApplied = &HandleDamageApplied;
    effectContext.actorBaseAtk = actor.atk;
    effectContext.actorTotalAtk = static_cast<float>(actor.atk);
    effectContext.resolveTargetDef = &ResolveTargetDef;
    ProcessId processId = DEBUG_PROCESS_BA_ID;
    switch (skillType)
    {
    case SkillType::BA:
        processId = DEBUG_PROCESS_BA_ID;
        break;
    case SkillType::BS:
        processId = DEBUG_PROCESS_BS_ID;
        break;
    case SkillType::SA:
        processId = DEBUG_PROCESS_SA_ID;
        break;
    case SkillType::Ability:
    default:
        processId = DEBUG_PROCESS_BA_ID;
        break;
    }
    if (!ExecuteProcess(processId, effectContext))
    {
        std::printf("[Process] Debug %s Process is not registered (ID=%u)\n",
                    skillType == SkillType::BS ? "BS" : "BA",
                    static_cast<unsigned int>(processId));
    }

    // Action Log に記録 (スロット位置形式: A1~A5, E1~E5、先頭に累積時刻 t を付加)
    char logEntry[128];
    char actorSlotStr[16], targetSlotStr[16];
    if (actorTeam == 1) {
        std::snprintf(actorSlotStr, sizeof(actorSlotStr), "A%d", actorSlot + 1);
    } else {
        std::snprintf(actorSlotStr, sizeof(actorSlotStr), "E%d", actorSlot + 1);
    }
    if (targetTeam == 1) {
        std::snprintf(targetSlotStr, sizeof(targetSlotStr), "A%d", targetSlot + 1);
    } else {
        std::snprintf(targetSlotStr, sizeof(targetSlotStr), "E%d", targetSlot + 1);
    }
    const char* skillTypeName =
        (skillType == SkillType::SA) ? "SA" :
        (skillType == SkillType::BS) ? "BS" : "BA";
    // 表示用時刻は小数第3位以下を切り捨てる（四捨五入しない）。
    // float の小数表現誤差で 68.27f が 68.269996... となる場合を考慮し、
    // float の半ULP程度を補正してから小数第2位で切り捨てる。
    const double truncatedTime =
        std::floor((static_cast<double>(g_accumTime) + 0.000004) * 100.0) / 100.0;
    std::snprintf(logEntry, sizeof(logEntry), "%s%s, %s (t = %.2f)", actorSlotStr, targetSlotStr, skillTypeName, truncatedTime);
    g_actionLog.push_back(logEntry);

    // Actor の AV を 10000/spd に更新
    for (auto& node : g_timeline)
    {
        if (node.team == actorTeam && node.slotIndex == actorSlot)
        {
            node.av = 10000.0f / static_cast<float>(actorSpd);
            break;
        }
    }

    // ソート
    SortTimeline();

    // 次の Actor を決定 (min AV 減算)
    if (!g_timeline.empty())
    {
        const float minAV = g_timeline[0].av;
        g_accumTime += minAV;
        for (auto& node : g_timeline)
        {
            node.av -= minAV;
        }
        g_timeline[0].av = 0.0f;
        g_actorTeam = g_timeline[0].team;
        g_actorSlot = g_timeline[0].slotIndex;
    }

    // ターゲット選択をリセット
    g_selectedTargetTeam = -1;
    g_selectedTargetSlot = -1;
}
