/**
 * @file simulationLogic.cpp
 * @brief 戦闘シミュレーションのロジック・タイムライン進行・履歴管理の実装。
 */

#include "simulationLogic.hpp"

#include <algorithm>
#include <array>
#include <cstdio>
#include <cmath>
#include <limits>
#include <utility>

namespace
{
    TeamState g_teamState{};
    SkillType g_currentSkillType = SkillType::BA;

    void handleDamageApplied(void* context, int team, int slot,
                             bool* allySlotOccupied, bool* enemySlotOccupied);

    /**
     * @brief 対象キオクの現在HPポインタを解決します。
     * @param context チーム状態へのポインタ（TeamState*）。
     * @param team チーム番号（0 = enemy, 1 = ally）。
     * @param slot スロット番号（0～4）。
     * @return 対象HPへのポインタ。不正時はnullptr。
     */
    int* resolveTargetHP(void* context, int team, int slot)
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
    float resolveTargetDef(void* context, int team, int slot)
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

    /**
     * @brief 対象キオクの加算固定値型SPD効果合計へのポインタを解決します。
     * @param context チーム状態へのポインタ（TeamState*）。
     * @param team チーム番号（0 = enemy, 1 = ally）。
     * @param slot スロット番号（0～4）。
     * @return 加算固定値型SPD効果の合計へのポインタ。不正時はnullptr。
     */
    int* resolveTargetSPDFixedBonus(void* context, int team, int slot)
    {
        if (context == nullptr || slot < 0 || slot >= 5 ||
            (team != 0 && team != 1))
        {
            return nullptr;
        }

        auto* state = static_cast<TeamState*>(context);
        return (team == 1)
            ? &state->ally[slot].spdFixedBonus
            : &state->enemy[slot].spdFixedBonus;
    }

    /**
     * @brief 対象キオクのSPDが変化した時にAVおよびPriorityの更新を行います。
     * @details 速度変化時は AV = (oldSpeed / newSpeed) * AV を float で行う。
     *          totalSPD が変わらない再付与でもPriorityは更新される。上式が通っているかは不明だが、
     *          効果の消去&再付与は否定されることと、誤差を生じ得ないことから再付与の場合スルーしている。
     * @param context チーム状態へのポインタ（TeamState*）。
     * @param team チーム番号（0 = enemy, 1 = ally）。
     * @param slot スロット番号（0～4）。
     * @param priority 発行済みのPriority番号。
     */
    void handleTargetSPDChanged(void* context, int team, int slot,
                                 std::uint32_t priority)
    {
        if (context == nullptr || slot < 0 || slot >= 5 ||
            (team != 0 && team != 1))
        {
            return;
        }

        auto* state = static_cast<TeamState*>(context);
        UnitStatDetails& unit = (team == 1)
            ? state->ally[slot]
            : state->enemy[slot];

        rescaleAVBySpeed(unit, calcTotalSPD(unit));
        unit.turnPriority = priority;
    }
}

// ===========================================================
// AV・SPD 処理
// ===========================================================

/**
 * @brief ユニットの現在のtotalSPDを計算します。
 * @details [現行: 加算固定値型のみ実装] 基礎SPDへBuffSPDFixedの合計値を返す。
 *			乗算型SPD効果（SPD-）との2パス合成予定。
 *			状態エンジン実装時にこの関数を置換する。
 */
float calcTotalSPD(const UnitStatDetails& unit)
{
	// TODO: 乗算型SPD効果と2パス合成を実装する。
    int totalSPD = std::max(0, unit.character.spd + unit.spdFixedBonus);
    return totalSPD;
}

/**
 * @brief AVにその時点でのTurnAVを代入します（TL初期化 / 行動開始時のAV計算）。
 * @details calcTotalSPD -> turnPriority = 0; -> av = 10000 / totalSPD の順で処理する。
 *			AV値が同じノードが既にTLに存在するとき、そのノードより後方に配置される。
 */
void setTurnAV(UnitStatDetails& unit)
{
    unit.totalSPD = calcTotalSPD(unit);
    unit.turnPriority = 0;
    unit.av = 10000.0f / unit.totalSPD;
}

/**
 * @brief SPD変化時のAV再スケールを行います。
 * @details SPDに何も変化がなければ何も処理しない。また、
 *          Priority の更新はこの関数では行わない（効果適用側で
 *          nextTurnOrderPriority() を呼ぶ）。
 */
void rescaleAVBySpeed(UnitStatDetails& unit, float newSPD)
{
    if (unit.totalSPD != newSPD)
    {
        unit.av = (unit.totalSPD / newSPD) * unit.av;
        unit.totalSPD = newSPD;
    }
}

/**
 * @brief 行動順上昇/減少効果（AA/AD）によるAV更新を行います。
 * @details turnAVを求め、それの割合 rate を対象の AV に加え、0との最大値をとる。
 *          行動順上昇は rate < 0、行動順減少は rate > 0。
 *          rate と priority は各効果適用につき1度だけ計算し、全対象で同一の値を使う。
 */
void shiftAVByActionOrder(UnitStatDetails& unit, float rate, std::uint32_t priority)
{
    const float turnAV = 10000.0f / unit.totalSPD;
    const float nextAV = (turnAV * rate) + unit.av;
    unit.av = (nextAV <= 0.0f) ? 0.0f : nextAV;
    unit.turnPriority = priority;
}

/**
 * @brief 次のターン優先権番号を発行します。
 * @details 効果適用1回につき1度発行する。発行した値を、その効果の対象に
 *          与える（AVが変わりえない効果では呼ばれない）。
 */
std::uint32_t nextTurnOrderPriority(TeamState& team)
{
    return team.nextTurnPriority++;
}

namespace
{
    /**
     * @brief 履歴スナップショット構造体。
     */
    struct HistorySnapshot
    {
        TeamState teamState;						/**< チーム状態 */
        std::vector<TimelineNode> timeline;			/**< タイムライン配列 */
        int actorTeam = -1;							/**< Actorのチーム */
        int actorSlot = -1;							/**< Actorのスロット */
        int selectedTargetTeam = -1;				/**< 選択された対象のチーム */
        int selectedTargetSlot = -1;				/**< 選択された対象のスロット */
        float accumTime = 0.0f;						/**< 累積時間 */
		SkillType currentSkillType = SkillType::BA; /**< 選択した行動 */
		
    };
}

// ===========================================================
// タイムライン
// ===========================================================

static std::vector<TimelineNode> g_timeline;

// 現在の Actor (タイムライン先頭)
static int g_actorTeam = -1;     // 0=enemy, 1=ally, -1=none
static int g_actorSlot = -1;

// 選択中のターゲット
static int g_selectedTargetTeam = -1;
static int g_selectedTargetSlot = -1;

// 累積時刻 t (minAV 減算の累積値)
static float g_accumTime = 0.0f;

/**
 * @brief 各履歴ステップで実行された行動情報。
 */
struct HistoryAction
{
    SkillType skillType = SkillType::BA; /**< スキル種別 */
    int targetTeam = -1;                 /**< ターゲットチーム */
    int targetSlot = -1;                 /**< ターゲットスロット */
};

namespace
{
    /**
     * @brief 分岐の辺（あるノードから出る1つの行動選択）。
     */
    struct BranchEdge
    {
        HistoryAction action;  /**< 実行された行動 */
        int childNode = -1;    /**< 行動適用後の状態ノードID */
        std::string text;      /**< Action Log 行テキスト（累積時刻付き） */
        std::string shortText; /**< Branch パネル行テキスト（累積時刻なし） */
    };

    /**
     * @brief 分岐木のノード。子行動は作成順で保持する。
     */
    struct HistoryNode
    {
        HistorySnapshot snapshot;         /**< その時点の盤面スナップショット */
        std::vector<BranchEdge> children; /**< 出る子行動の列（作成順） */
    };
}

// 履歴管理（分岐木）。ノードID = g_historyNodes の添字。ノード0 = バトル開始時の状態。
static std::vector<HistoryNode> g_historyNodes;

// Action Log / Branch パネルの表示対象ライン（ルートから葉までのノードID列）。
// 分岐への遷移（Branch パネル行クリック／新分岐作成／別分岐の子行動と一致する行動の実行）で
// 現在位置から先が新たな分岐へ組み替えられ、その先の分岐点では最も古く作成された
// 子行動（作成順の先頭）で延長される。Undo/Redo・Action Log の行クリックはこのラインに沿って移動する。
static std::vector<int> g_displayPath;

// 現在表示している状態のインデックス（g_displayPath 上の位置）。
// 0 = 初期状態
static int g_historyIndex = 0;

// 描画用の行キャッシュ（復元・進行・分岐切替時に dirty になり遅延再構築）
static std::vector<HistoryLineRow> g_lineRows;
static std::vector<HistoryBranchRow> g_branchRows;
static bool g_lineRowsDirty = true;
static bool g_branchRowsDirty = true;

namespace
{
    /**
     * @brief 付与効果・被付与効果の解除処理（将来実装用）。
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
    void handleDamageApplied(void* context, int team, int slot,
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

        // 戦闘不能になったユニットのMP, SP, 与えている効果、受けている効果をすべて解除する。SPはパーティ共有値なので変更しない。
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

        // Actor が戦闘不能になった場合 (持続ダメージ等)
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
 * @param actorTeam Actorのチーム。
 * @param actorSlot Actorのスロット番号。
 * @param targetTeam 対象のチーム。
 * @param targetSlot 対象のスロット番号。
 * @return 行動ログ行テキスト（TLが空の場合は空文字列）。
 */
static std::string proceedTurn(SkillType skillType, int actorTeam, int actorSlot, int targetTeam, int targetSlot);

/**
 * @brief 現在のチーム状態への参照を取得します。
 */
const TeamState& getSimulationTeamState()
{
    return g_teamState;
}

/**
 * @brief ロジック側のチーム状態を設定・初期化します。
 */
void setSimulationTeamStateLogic(const TeamState& state)
{
    g_teamState = state;

    for (int i = 0; i < 5; ++i)
    {
        for (UnitStatDetails* slot : { &g_teamState.ally[i], &g_teamState.enemy[i] })
        {
            if (slot->character.id != 0)
            {
                slot->hp = std::max(0, slot->hp);
                // 戦闘開始時点のHPを最大HPとする
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

    initTimeline();
}

/**
 * @brief 現在のタイムラインを取得します。
 */
const std::vector<TimelineNode>& getTimeline()
{
    return g_timeline;
}

/**
 * @brief 表示中のライン（Action Log の表示対象経路）に沿った行一覧を取得します。
 * @details 行 i は「経路上のノード i から出る行動」に対応する。
 *          遷移元ノードに複数の子行動がある行には branch が立つ（"+" 表示用）。
 * @return 行情報配列へのconst参照。
 */
const std::vector<HistoryLineRow>& getHistoryLineRows()
{
    if (g_lineRowsDirty)
    {
        g_lineRows.clear();
        if (g_displayPath.size() >= 1)
        {
            g_lineRows.reserve(g_displayPath.size() - 1);
        }
        for (std::size_t i = 0; i + 1 < g_displayPath.size(); ++i)
        {
            const int parentNodeId = g_displayPath[i];
            const int childNodeId = g_displayPath[i + 1];
            const std::vector<BranchEdge>& edges =
                g_historyNodes[parentNodeId].children;
            for (const BranchEdge& edge : edges)
            {
                if (edge.childNode != childNodeId)
                {
                    continue;
                }
                HistoryLineRow row;
                row.text = edge.text;
                row.branch = (edges.size() > 1);
                g_lineRows.push_back(std::move(row));
                break;
            }
        }
        g_lineRowsDirty = false;
    }
    return g_lineRows;
}

/**
 * @brief 現在位置ノードから出ている子行動（Branch パネルの行）一覧を取得します。
 * @details 並びは作成順。現在の表示ラインが通る子行動には selected が立つ。
 * @return 行情報配列へのconst参照。
 */
const std::vector<HistoryBranchRow>& getHistoryBranchRows()
{
    if (g_branchRowsDirty)
    {
        g_branchRows.clear();
        if (g_historyIndex >= 0 &&
            g_historyIndex < static_cast<int>(g_displayPath.size()))
        {
            const int positionNodeId = g_displayPath[g_historyIndex];
            const std::vector<BranchEdge>& edges =
                g_historyNodes[positionNodeId].children;
            // 分岐が確定していない（子行動が1つ以下の）ノードは行を表示しない
            if (edges.size() >= 2)
            {
                for (const BranchEdge& edge : edges)
                {
                    HistoryBranchRow row;
                    row.text = edge.shortText;
                    row.selected =
                        (g_historyIndex + 1 < static_cast<int>(g_displayPath.size()) &&
                         edge.childNode == g_displayPath[g_historyIndex + 1]);
                    g_branchRows.push_back(std::move(row));
                }
            }
        }
        g_branchRowsDirty = false;
    }
    return g_branchRows;
}

/**
 * @brief 指定ノード以降の表示ラインを、最も古く作成された子行動（作成順の先頭）で延長します。
 * @param nodeId 延長起点のノードID（すでに g_displayPath に積まれている前提）。
 */
static void extendDisplayPathFromNode(int nodeId)
{
    while (true)
    {
        const std::vector<BranchEdge>& childEdges =
            g_historyNodes[nodeId].children;
        if (childEdges.empty())
        {
            break;
        }
        nodeId = childEdges.front().childNode;
        g_displayPath.push_back(nodeId);
    }
}

/**
 * @brief 指定ノードのスナップショット内容を現在の状態へ復元します。
 * @param nodeId 復元先ノードID。
 */
static void restoreNodeState(int nodeId)
{
    const HistorySnapshot& snap = g_historyNodes[nodeId].snapshot;

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

    // 選択した行動を復元
    g_currentSkillType = snap.currentSkillType;
}

/**
 * @brief Branch パネルで表示された子行動を選択します（行クリック時）。
 * @details 選択した子行動の適用後ノードへ盤面を復元し、Action Log の表示ラインも
 *          その分岐へ切り替える。表示先の更なる分岐点は作成順の最古の子で延長する。
 * @param row 子行動の作成順インデックス。
 */
void selectHistoryBranch(int row)
{
    if (g_historyIndex < 0 ||
        g_historyIndex >= static_cast<int>(g_displayPath.size()))
    {
        return;
    }
    const int positionNodeId = g_displayPath[g_historyIndex];
    const std::vector<BranchEdge>& edges =
        g_historyNodes[positionNodeId].children;
    if (row < 0 || row >= static_cast<int>(edges.size()))
    {
        return;
    }

    const int childNodeId = edges[row].childNode;

    // 表示ラインを現在位置まで切り、選択した分岐へ組み替える
    g_displayPath.resize(g_historyIndex + 1);
    g_displayPath.push_back(childNodeId);
    extendDisplayPathFromNode(childNodeId);

    restoreNodeState(childNodeId);
    g_historyIndex += 1;
    g_lineRowsDirty = true;
    g_branchRowsDirty = true;
}

/**
 * @brief 現在のActorのチームを取得します。
 */
int getActorTeam() { return g_actorTeam; }

/**
 * @brief 現在のActorのスロットインデックスを取得します。
 */
int getActorSlot() { return g_actorSlot; }

/**
 * @brief 戦闘開始からの累積時間を取得します。
 */
float getAccumTime() { return g_accumTime; }

/**
 * @brief 表示中のライン上の現在位置インデックスを取得します。
 * @return 履歴インデックス（0 = 初期状態）。
 */
int getHistoryIndex() { return g_historyIndex; }

/**
 * @brief 表示中のラインのノード総数を取得します。
 * @return ライン上のノード数（行数 + 1）。
 */
int getHistoryCount() { return static_cast<int>(g_displayPath.size()); }

/**
 * @brief 現在選択されているスキル種別を取得します。
 */
SkillType getCurrentSkillType() { return g_currentSkillType; }

/**
 * @brief 指定したスキル種別が現在選択可能かを返します。
 * @details SAは常時不可。BSは現在のActorの陣営SPが0のとき不可
 *          （Actor未決定時はSP検査をしない）。BAは常時可。
 */
bool isSkillTypeSelectable(SkillType type)
{
    if (type == SkillType::SA) return false;
    if (type == SkillType::BS && g_actorTeam >= 0)
    {
        const int teamIndex = (g_actorTeam == 1) ? 1 : 0;
        if (g_teamState.sp[teamIndex] <= 0) return false;
    }
    return true;
}

/**
 * @brief 現在選択されているスキル種別を切り替えます（BA <-> BS）。
 * @details 切替先が選択不可（SP0時のBS等）なら選択状態を変更しない。
 */
void toggleCurrentSkillType()
{
    const SkillType next = (g_currentSkillType == SkillType::BA)
        ? SkillType::BS
        : SkillType::BA;
    if (isSkillTypeSelectable(next)) g_currentSkillType = next;
}

/**
 * @brief TLの2ノード(a, b)を受け取り、aが先行なら-1、そうでないなら1を返します。
 * @details ノードの先行判断は以下による整列による:
 *  1) AV 昇順
 *  2) turnPriority 降順
 *  3) チーム順 (ally -> enemy）
 *  4) slotIndex 昇順
 * スロットはチーム内で一意のため全キー一致は構造上発生しない。
 */
static int compareTurnOrder(const TeamState& state, const TimelineNode& a, const TimelineNode& b)
{
    const UnitStatDetails& unitA =
        (a.team == 1) ? state.ally[a.slotIndex] : state.enemy[a.slotIndex];
    const UnitStatDetails& unitB =
        (b.team == 1) ? state.ally[b.slotIndex] : state.enemy[b.slotIndex];

	// 1. AVが小さい方が先行
    if (unitA.av != unitB.av)
		return (unitA.av < unitB.av) ? -1 : 1;
	
	// 2. turnPriorityが大きい方が先行
    if (unitA.turnPriority != unitB.turnPriority)
		return (unitA.turnPriority > unitB.turnPriority) ? -1 : 1;
	
	// 3. チームが異なるとき、味方側が先行 (TL初期化用)
    if (a.team != b.team)
		return (a.team > b.team) ? -1 : 1;
	
	// 4. slotIndexが小さい方が先行
    return (a.slotIndex < b.slotIndex) ? -1 : 1;
}

/**
 * @brief タイムラインを整列します（N<=10のほぼ整列済み配列への Insertion）。
 * @details shiftNextTurn 後はfloatの丸め衝突の可能性があるため
 *          全体ソートで整列し直す。単一ユニットの変化は relocateTimelineNode を使う。
 */
static void sortTimeline()
{
    for (int i = 1; i < static_cast<int>(g_timeline.size()); ++i)
    {
        const TimelineNode key = g_timeline[i];
        int j = i - 1;
        while (j >= 0 && compareTurnOrder(g_teamState, key, g_timeline[j]) < 0)
        {
            g_timeline[j + 1] = g_timeline[j];
            --j;
        }
        g_timeline[j + 1] = key;
    }
}

/**
 * @brief 単一ユニットの AV / Priority 変化を整列位置へ反映します（方式: 削除+挿入）。
 * @details setTurnAV / shiftAVByActionOrder / rescaleAVBySpeed の後など、
 *          1ユニットだけが変化した場合に全体ソート（O(n^2)）を避けたいため。TLが整列済みで
 *          ある限り、1ノードの削除+挿入で整列は保存される。
 */
static void relocateTimelineNode(int team, int slotIndex)
{
    int index = -1;
    for (int i = 0; i < static_cast<int>(g_timeline.size()); ++i)
    {
        if (g_timeline[i].team == team && g_timeline[i].slotIndex == slotIndex)
        {
            index = i;
            break;
        }
    }
    if (index < 0)
    {
        return;
    }

    const TimelineNode node = g_timeline[index];
    g_timeline.erase(g_timeline.begin() + index);

    int insertPos = static_cast<int>(g_timeline.size());
    for (int i = 0; i < static_cast<int>(g_timeline.size()); ++i)
    {
        if (compareTurnOrder(g_teamState, node, g_timeline[i]) < 0)
        {
            insertPos = i;
            break;
        }
    }
    g_timeline.insert(g_timeline.begin() + insertPos, node);
}

/**
 * @brief 先頭ユニットの AV を全ユニットから引きます。
 * @details AV = max(0, AV − minAV)。この関数は「行動終了後・次の行動開始前」のタイミングで
 *          呼ばれ、処理後、全体ソートで整列を取り直すようにしている。
 *          minAV 減算によって通常大小逆転は発生しないが、float の丸めにより異なる値が
 *          同じ値になることがある。 sortTimeline はこのようなノードに対し、4つの変数による
 *			整列に直す処理をとっているが、実機で同じ処理になっているかは不明。
 */
static void shiftNextTurn(float minAV)
{
    for (const TimelineNode& node : g_timeline)
    {
        UnitStatDetails& unit = (node.team == 1)
            ? g_teamState.ally[node.slotIndex]
            : g_teamState.enemy[node.slotIndex];
        unit.av = std::max(0.0f, unit.av - minAV);
    }
    sortTimeline();
	g_accumTime += minAV;
}

/**
 * @brief ユニットの行動値（UnitStatDetails.av）を取得します。
 */
static float resolveUnitAV(const TeamState& state, int team, int slotIndex)
{
    return (team == 1) ? state.ally[slotIndex].av : state.enemy[slotIndex].av;
}

/**
 * @brief 現在の状態をスナップショットとして新しいノードを履歴に追加します。
 * @return 作成したノードのID。
 */
static int createSnapshotNode()
{
    HistoryNode node;
    node.snapshot.teamState = g_teamState;
    node.snapshot.timeline = g_timeline;
    node.snapshot.actorTeam = g_actorTeam;
    node.snapshot.actorSlot = g_actorSlot;
    node.snapshot.selectedTargetTeam = g_selectedTargetTeam;
    node.snapshot.selectedTargetSlot = g_selectedTargetSlot;
    node.snapshot.accumTime = g_accumTime;
	node.snapshot.currentSkillType = g_currentSkillType;
    g_historyNodes.push_back(std::move(node));
    return static_cast<int>(g_historyNodes.size()) - 1;
}

/**
 * @brief 指定された表示ライン上の位置における盤面を復元します。
 */
void restoreHistoryStep(int index)
{
    if (index < 0 ||
        index >= static_cast<int>(g_displayPath.size()))
    {
        return;
    }

    restoreNodeState(g_displayPath[index]);

    // 現在表示している状態のインデックス
    g_historyIndex = index;
    g_lineRowsDirty = true;
    g_branchRowsDirty = true;
}

/**
 * @brief バトル開始時のアビリティ効果を指定した順序で適用します。
 * @details 適用順序:
 *   1. 味方 → 敵 の順。
 *   2. チーム内はスロット順（Slot1 → 5）で1体ずつ。
 *   3. 同一ユニットのPassive効果は battleStartPriority 降順
 *     （効果値変化系 1 / その他 0。バフ効果量のため）。
 *   - 対象はタイムライン上のユニットのみ（戦闘不能・空スロットはスキップ）。
 *   - SPD系効果の適用1回につき nextTurnOrderPriority() を呼び出し、効果の
 *     対象にその時点でのPriority値を与える。
 * 現在はデバッグ用アビリティ（スピード+1 / BuffSPDFixed / Range: Self）のみを
 * 実行する。Effect列の取得と battleStartPriority 降順の stable sort、
 * および複数Effectの連結は未実装（TODO）。
 * 初期整列は後続の sortTimeline() が担保する。
 */
static void applyBattleStartAbility()
{
	// スロットの有効性の判定
    bool allySlotOccupied[5]{};
    bool enemySlotOccupied[5]{};
    for (int i = 0; i < 5; ++i)
    {
        allySlotOccupied[i] = (g_teamState.ally[i].character.id != 0 &&
                               g_teamState.ally[i].hp > 0);
        enemySlotOccupied[i] = (g_teamState.enemy[i].character.id != 0 &&
                                g_teamState.enemy[i].hp > 0);
    }

    // チーム1（味方）→ チーム0（敵）。チーム内はスロット順（Slot1 → 5）。
    for (const int team : { 1, 0 })
    {
        for (int slotIndex = 0; slotIndex < 5; ++slotIndex)
        {
            const UnitStatDetails& unit = (team == 1)
                ? g_teamState.ally[slotIndex]
                : g_teamState.enemy[slotIndex];
            if (unit.character.id == 0 || unit.hp <= 0)
            {
                continue; // 空スロット・戦闘不能の場合スキップ
            }

            // 効果適用1回につき優先権を1度発行し、その値を効果の対象へ与える。
            // SPDが変わらないSPD効果の再付与でもPriorityは発行・更新される。
            const std::uint32_t priority = nextTurnOrderPriority(g_teamState);

            EffectContext effectContext{};
            effectContext.actorTeam = team;
            effectContext.actorSlot = slotIndex;
            effectContext.targetTeam = team;
            effectContext.targetSlot = slotIndex;
            effectContext.allySlotOccupied = allySlotOccupied;
            effectContext.enemySlotOccupied = enemySlotOccupied;
            effectContext.spdContext = &g_teamState;
            effectContext.resolveTargetSPDFixedBonus = &resolveTargetSPDFixedBonus;
            effectContext.onTargetSPDChanged = &handleTargetSPDChanged;
            effectContext.turnPriority = priority;

            // TODO(戦闘システム): unit の戦闘開始アビリティ効果に紐づく Effect列を
            // battleStartPriority 降順 stable sort で取得し、1件ずつ適用する。
            // 現在はデバッグ用アビリティ（スピード+1 / Increase_SPD_Fixed /
            // Range Self）の1件のみを適用する。
			
			// [DEBUG]
			// DEBUG_PROCESS_ABILITY_IDが実行されなかった場合、ログを出力する。
            if (!ExecuteProcess(DEBUG_PROCESS_ABILITY_ID, effectContext))
            {
                std::printf("[Process] Debug Ability Process is not registered (ID=%u)\n",
                            static_cast<unsigned int>(DEBUG_PROCESS_ABILITY_ID));
            }

            // AV・Priority の変化を整列へ反映する。
            relocateTimelineNode(team, slotIndex);
        }
    }
}

/**
 * @brief タイムラインの初期化および初期Actorの解決を行います。
 * @details バトル開始時の処理順序:
 *  1) 全ユニットのAVを setTurnAVで計算、Priority 0
 *  2) TL 構築
 *  3) バトル開始時アビリティ効果の適用
 *  4) 初期整列
 *  5) shiftNextTurn: 全員から TL 先頭の AV を引き、先頭を Actor とする。
 */
void initTimeline()
{
	
	// TL・行動履歴等 をクリア。
	// TODO: formationView時にファイル読み込みで、履歴を再現する処理の追加 (優先度低)
    g_timeline.clear();
    // 追加行動を考慮した確保
    g_timeline.reserve(16);
    g_actorTeam = -1;
    g_actorSlot = -1;
    g_selectedTargetTeam = -1;
    g_selectedTargetSlot = -1;
	g_accumTime = 0.0f;
	g_currentSkillType = SkillType::BA;

    // ノード構築（敵チーム team=0、味方チーム team=1。空スロット・HP0を除外）
    for (int team = 0; team < 2; ++team)
    {
        for (int i = 0; i < 5; ++i)
        {
            const UnitStatDetails& slot =
                (team == 0) ? g_teamState.enemy[i] : g_teamState.ally[i];
            if (slot.character.id != 0 && slot.hp > 0)
            {
                TimelineNode node{};
                node.team = team;
                node.slotIndex = i;
                g_timeline.push_back(node);
            }
        }
    }

    // バトル開始時の初期AV（Priority 0、AV = TurnAV）
    for (const TimelineNode& node : g_timeline)
    {
        UnitStatDetails& unit = (node.team == 1)
            ? g_teamState.ally[node.slotIndex]
            : g_teamState.enemy[node.slotIndex];
        setTurnAV(unit);
    }

    applyBattleStartAbility();

    sortTimeline();

    // minAV を減算して先頭の Actor を決定
    if (!g_timeline.empty())
    {
        const float minAV = resolveUnitAV(g_teamState,
            g_timeline[0].team, g_timeline[0].slotIndex);
        shiftNextTurn(minAV);
        g_actorTeam = g_timeline[0].team;
        g_actorSlot = g_timeline[0].slotIndex;
    }
    // 履歴を初期化 — 分岐木と表示ラインをクリアし、初期状態をルートに保存
    g_historyNodes.clear();
    g_displayPath.clear();
    const int rootNodeId = createSnapshotNode();
    g_displayPath.push_back(rootNodeId);
    g_historyIndex = 0;
    g_lineRowsDirty = true;
    g_branchRowsDirty = true;
}

/**
 * @brief 履歴を1ステップ前に巻き戻します（Undo）。
 */
void backupTurn()
{
    if (g_historyIndex <= 0)
    {
        return;
    }

    restoreHistoryStep(g_historyIndex - 1);
}

/**
 * @brief 履歴を1ステップ進めます（Redo）。
 */
void forwardTurn()
{
    const int lastIndex =
        static_cast<int>(g_displayPath.size()) - 1;

    if (g_historyIndex >= lastIndex)
    {
        return;
    }

    restoreHistoryStep(g_historyIndex + 1);
}

/**
 * @brief スキル実行およびターゲット選択を伴い、履歴を記録・分岐してターンを進めます。
 * @details 既存の子行動と同じ行動を選んだ場合はそのノードへ復元して再生する
 *          （表示ライン上の子ならそのまま進み、別分岐の子なら Branch 行クリックと
 *          同じ動作で表示ラインを切り替える）。どの子行動とも一致しない場合は
 *          新しく行動を実行して分岐木にノードを追加する。現在位置ノードに既に
 *          子行動がある場合は新分岐の作成となり、Action Log の表示ラインも
 *          新分岐へ切り替わる。
 */
void proceedTurnWithHistory(SkillType skillType, int targetTeam, int targetSlot)
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
    if (g_historyIndex < 0 ||
        g_historyIndex >= static_cast<int>(g_displayPath.size()))
    {
        return;
    }

    const int positionNodeId = g_displayPath[g_historyIndex];

    // -------------------------------------------------------
    // 既存の子行動と同じ行動を選んだ場合:
    // 表示ライン上の子ならそのまま進み、別分岐の子なら
    // Branch 行クリックと同じ動作（盤面復元＋表示切替）を行う
    // -------------------------------------------------------
    {
        const std::vector<BranchEdge>& edges =
            g_historyNodes[positionNodeId].children;
        for (std::size_t i = 0; i < edges.size(); ++i)
        {
            const BranchEdge& edge = edges[i];
            if (edge.action.skillType == skillType &&
                edge.action.targetTeam == targetTeam &&
                edge.action.targetSlot == targetSlot)
            {
                if (g_historyIndex + 1 < static_cast<int>(g_displayPath.size()) &&
                    edge.childNode == g_displayPath[g_historyIndex + 1])
                {
                    restoreHistoryStep(g_historyIndex + 1);
                }
                else
                {
                    selectHistoryBranch(static_cast<int>(i));
                }
                return;
            }
        }
    }

    // -------------------------------------------------------
    // 既存のどの子行動とも一致しない → 新しく実行する
    // -------------------------------------------------------

    g_selectedTargetTeam = targetTeam;
    g_selectedTargetSlot = targetSlot;

    // 行動テキスト用のスロット位置表記。
    // proceedTurn 内で Actor が死亡した場合 g_actorTeam/g_actorSlot が
    // 更新されうるため、実行前に作成する。
    char actorSlotStr[16], targetSlotStr[16];
    if (g_actorTeam == 1) {
        std::snprintf(actorSlotStr, sizeof(actorSlotStr), "A%d", g_actorSlot + 1);
    } else {
        std::snprintf(actorSlotStr, sizeof(actorSlotStr), "E%d", g_actorSlot + 1);
    }
    if (targetTeam == 1) {
        std::snprintf(targetSlotStr, sizeof(targetSlotStr), "A%d", targetSlot + 1);
    } else {
        std::snprintf(targetSlotStr, sizeof(targetSlotStr), "E%d", targetSlot + 1);
    }
    const char* skillTypeName = (skillType == SkillType::BS) ? "BS" : "BA";

    const std::string actionText = proceedTurn(
        skillType,
        g_actorTeam,
        g_actorSlot,
        targetTeam,
        targetSlot
    );

    // 行動適用後の状態を新しいノードとして保存
    const int newNodeId = createSnapshotNode();

    BranchEdge edge;
    edge.action.skillType = skillType;
    edge.action.targetTeam = targetTeam;
    edge.action.targetSlot = targetSlot;
    edge.childNode = newNodeId;
    edge.text = actionText;
    char shortText[32];
    std::snprintf(shortText, sizeof(shortText), "%s%s, %s",
                  actorSlotStr, targetSlotStr, skillTypeName);
    edge.shortText = shortText;

    // createSnapshotNode の push_back で参照が無効化しうるため取り直す
    HistoryNode& parentNode = g_historyNodes[positionNodeId];
    const bool hadChildren = !parentNode.children.empty();
    parentNode.children.push_back(std::move(edge));

    if (hadChildren)
    {
        // 分岐の作成: 表示ラインを現在位置まで切り、新分岐へ切り替える。
        // 新ノードは子を持たないためラインの終端は新ノード。
        g_displayPath.resize(g_historyIndex + 1);
        g_displayPath.push_back(newNodeId);
    }
    else
    {
        // 表示ラインの延長
        g_displayPath.insert(
            g_displayPath.begin() + (g_historyIndex + 1), newNodeId);
    }

    // 最新の状態へ移動
    g_historyIndex += 1;
    g_lineRowsDirty = true;
    g_branchRowsDirty = true;
}

/**
 * @brief ターンを1回分進行させ、スキル実行・AV更新・ログ記録を行います。
 * @return 行動ログ行テキスト（TLが空の場合は空文字列）。
 */
static std::string proceedTurn(SkillType skillType, int actorTeam, int actorSlot, int targetTeam, int targetSlot)
{
    if (g_timeline.empty()) return std::string();

    UnitStatDetails& actor = (actorTeam == 1)
        ? g_teamState.ally[actorSlot]
        : g_teamState.enemy[actorSlot];

    // 行動開始時の AV を計算し、TL に挿入する。
    setTurnAV(actor);
    relocateTimelineNode(actorTeam, actorSlot);

    const char* actorName = actor.character.name;

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

    // BA/BS実行前処理：SPとMP/BPを更新する。
    // SPは陣営ごとに独立し、Actorの陣営のSPだけを進める。
    // BA: SP+1（上限6） / BS: SP-1（BSはActorの陣営SP>=1のときのみ選択可能）。
    // MPキオク: BAでMP+15、BSでMP+30。BPキオク（BP>0）: BP+1、BP+2。
    // currentMP / currentBP はそれぞれ最大値（UnitStatDetails::MP / BP）を上限とする。
    const int actorTeamIndex = (actorTeam == 1) ? 1 : 0;
	switch (skillType)
	{
		case SkillType::BA:
			g_teamState.sp[actorTeamIndex] = std::min(g_teamState.sp[actorTeamIndex] + 1, 6);
			if (actor.BP > 0 )
			{
				actor.currentBP = std::min(actor.currentBP + 1, actor.BP);
			}
			else
			{
				actor.currentMP = std::min(actor.currentMP + 15, actor.MP);
			}
			break;
		
		case SkillType::BS:
			// 選択ゲート（isSkillTypeSelectable）でSP>=1を満たす前提
			g_teamState.sp[actorTeamIndex] -= 1;
			if (actor.BP > 0)
			{
				actor.currentBP = std::min(actor.currentBP + 2, actor.BP);
			}
			else
			{
				actor.currentMP = std::min(actor.currentMP + 30, actor.MP);
			}
			break;
		default:
			break;
	}

    // ActorのキオクID・魔力解放からProcess IDを生成。
	// TODO: formation -> simulation移行時にこのID生成をしておく。
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
    effectContext.resolveTargetHP = &resolveTargetHP;
    effectContext.onDamageApplied = &handleDamageApplied;
    effectContext.actorBaseAtk = actor.atk;
    effectContext.actorTotalAtk = static_cast<float>(actor.atk);
    effectContext.resolveTargetDef = &resolveTargetDef;
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

    // 行動履歴 に記録 (スロット位置形式: A1~A5, E1~E5、先頭に累積時刻 t を付加)
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
    // 表示用時刻は小数第3位以下を切り捨てる。
    const float truncatedTime = std::floor(g_accumTime * 100.0f) / 100.0f;
    std::snprintf(logEntry, sizeof(logEntry), "%s%s, %s (t = %.2f)", actorSlotStr, targetSlotStr, skillTypeName, truncatedTime);
    std::string actionLogText(logEntry);

    // 次の Actor を決定する。
    // Process 実行中の死亡処理・行動順/SPD効果は AV と Priority を更新するため、
    // ここでの残処理は shiftNextTurn（減算＋全体ソート）のみ。先頭の AV
    // と同じ値を全員から引き、その後の TL の表示値が確定する。
    if (!g_timeline.empty())
    {
        const float nextAV = resolveUnitAV(g_teamState,
            g_timeline[0].team, g_timeline[0].slotIndex);
        shiftNextTurn(nextAV);
        g_actorTeam = g_timeline[0].team;
        g_actorSlot = g_timeline[0].slotIndex;
		if (g_teamState.sp[g_timeline[0].team] <= 0)
			g_currentSkillType = SkillType::BA;
    }

    // ターゲット選択をリセット
    g_selectedTargetTeam = -1;
    g_selectedTargetSlot = -1;

    return actionLogText;
}
