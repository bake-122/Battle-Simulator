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
	Character character{};	/**< キオクの静的データ */
	int hp = 0;				/**< 現在HP */
	int maxHP = 0;			/**< 戦闘開始時の最大HP */
	int atk = 0;			/**< 攻撃力 */
	int def = 0;			/**< 防御力 */
	int currentMP = 0;		/**< 現在MP */
	int currentBP = 0;		/**< 現在BP */
	
	int MP = 0;				/**< 最大MP */
	int BP = 0;				/**< 最大BP */

	// ------------------------------------------------------------
	// タイムラインに関するメンバー
	// ------------------------------------------------------------

	float av = 0.0f;					/**< 行動値 (Action Value) */
	float totalSPD = 0.0f;				/**< ゲージ基準の総SPD（setTurnAV / rescaleAVBySpeed で更新） */
	std::uint32_t turnPriority = 0;		/**< av が同値になったときのターン優先度（値が大きいほどTL先頭） */
};

/**
 * @brief 戦闘全体のチーム状態（味方・敵・陣営別SP）。
 */
struct TeamState
{
	std::array<UnitStatDetails, 5> ally{};	/**< 味方5枠 */
	std::array<UnitStatDetails, 5> enemy{};	/**< 敵5枠 */
	int sp[2] = {5, 5};						/**< 陣営別共有SP（添字=チームID: [0]=enemy, [1]=ally。初期値5、範囲0～6） */
	std::uint32_t nextTurnPriority = 0;		/**< 次に発行するターン優先権番号 */
};

/**
 * @brief タイムライン上の1ノードを表す構造体。
 * @details ノードは整列順（(team, slotIndex) の参照ペア）のみを保持し、AV等の
 *		  実体は UnitStatDetails 側に一元管理する。
 *		  整列は simulationLogic の compareTurnOrder（AV/turnPriority/チーム/スロットの4キー）が担う。
 */
struct TimelineNode
{
	int team;		  /**< チーム（0 = enemy, 1 = ally） */
	int slotIndex;	 /**< スロットインデックス（0～4） */
};

/**
 * @brief 行動スキル種別。
 */
enum class SkillType : std::uint8_t
{
	SA = 0,	   /**< Special Attack（必殺技） */
	BS = 1,	   /**< Battle Skill （戦闘スキル）*/
	BA = 2,	   /**< Basic Attack（通常攻撃） */
	Ability = 3   /**< アビリティ */
};

/**
 * @brief 現在のチーム状態への参照を取得します。
 * @return チーム状態へのconst参照。
 */
const TeamState& getSimulationTeamState();

/**
 * @brief ロジック側のチーム状態を設定・初期化します。
 * @param state 設定するチーム状態。
 */
void setSimulationTeamStateLogic(const TeamState& state);

/**
 * @brief 現在のタイムラインを取得します。
 * @return タイムラインノード配列へのconst参照。
 */
const std::vector<TimelineNode>& getTimeline();

/**
 * @brief Action Log の表示ライン上の1行の情報。
 */
struct HistoryLineRow
{
    std::string text;    /**< 行テキスト（例: "A2E3, BS (t = 12.34)"） */
    bool branch = false; /**< 遷移元ノードに複数の子行動が存在する（分岐あり） */
};

/**
 * @brief Branch パネルに表示する1行の情報。
 */
struct HistoryBranchRow
{
    std::string text;     /**< 行テキスト（例: "A2E3, BS"。累積時刻は含まない） */
    bool selected = false;/**< 現在の表示ラインが通る子行動か */
};

/**
 * @brief 表示中のライン（Action Log の表示対象経路）に沿った行一覧を取得します。
 * @details 行 i は「経路上のノード i から出る行動」に対応する。
 * @return 行情報配列へのconst参照。
 */
const std::vector<HistoryLineRow>& getHistoryLineRows();

/**
 * @brief 現在位置ノードから出ている子行動（Branch パネルの行）一覧を取得します。
 * @details 並びは作成順。現在の表示ラインが通る子行動には selected が立つ。
 *          子行動が1つ以下のノード（分岐未確定）では空を返す。
 * @return 行情報配列へのconst参照。
 */
const std::vector<HistoryBranchRow>& getHistoryBranchRows();

/**
 * @brief Branch パネルで表示された子行動を選択します（行クリック時）。
 * @details 選択した子行動の適用後ノードへ盤面を復元し、Action Log の表示ラインも
 *          その分岐へ切り替える（表示先の更なる分岐点は作成順の最古の子で延長）。
 * @param row 子行動の作成順インデックス。
 */
void selectHistoryBranch(int row);

/**
 * @brief 現在のActorのチームを取得します。
 * @return Actorのチーム番号（0 = enemy, 1 = ally）。
 */
int getActorTeam();

/**
 * @brief 現在のActorのスロットインデックスを取得します。
 * @return スロットインデックス（0～4）。
 */
int getActorSlot();

/**
 * @brief 戦闘開始からの累積時間を取得します。
 * @return 累積時間（float）。
 */
float getAccumTime();

// -----------------------------------------------------------
// AV・SPD 処理。
// SPD・AO(行動順)効果を実装する際は必ずこれらの関数経由でAVを操作する。
// -----------------------------------------------------------

/**
 * @brief ユニットの現在のtotalSPDを計算します。
 * @details [現行: SPD効果の状態未実装] 基礎SPDのみを返す。
 *			原則2パス合成（加算的効果を基礎値へ適用順に反映し、続いて
 *			乗算的効果を合計値へ適用順に反映する。）でtotalSPDを計算し、
 *			最後に float へ変換する。状態エンジン実装時にこの関数を置換する。
 */
float calcTotalSPD(const UnitStatDetails& unit);

/**
 * @brief AVにその時点でのTurnAVを代入します（TL初期化 / 行動開始時のAV計算）。
 * @details calcTotalSPD -> turnPriority = 0; -> av = 10000 / totalSPD の順で処理する。
 *			AV値が同じノードが既にTLに存在するとき、そのノードより後方に配置される。
 */
void setTurnAV(UnitStatDetails& unit);

/**
 * @brief SPD変化時のAV再スケールを行います。
 * @details SPDに何も変化がなければ何も処理しない。また、
 *          Priority の更新はこの関数では行わない（効果適用側で
 *          nextTurnOrderPriority() を呼ぶ）。
 */
void rescaleAVBySpeed(UnitStatDetails& unit, float newSPD);

/**
 * @brief 行動順上昇/減少効果（AA/AD）によるAV更新を行います。
 * @details turnAVを求め、それの割合 rate を対象の AV に加え、0との最大値をとる。
 *          行動順上昇は rate < 0、行動順減少は rate > 0。
 *          rate と priority は各効果適用につき1度だけ計算し、全対象で同一の値を使う。
 */
void shiftAVByActionOrder(UnitStatDetails& unit, float rate, std::uint32_t priority);

/**
 * @brief 次のターン優先権番号を発行します。
 * @details 効果適用1回につき1度発行する。発行した値を、その効果の対象に
 *          与える（AVが変わりえない効果では呼ばれない）。
 */
std::uint32_t nextTurnOrderPriority(TeamState& team);

/**
 * @brief 表示中のライン上の現在位置インデックスを取得します。
 * @return 履歴インデックス（0 = 初期状態）。
 */
int getHistoryIndex();

/**
 * @brief 表示中のラインのノード総数を取得します。
 * @return ライン上のノード数（Action Log 行数 + 1）。
 */
int getHistoryCount();

/**
 * @brief 現在選択されているスキル種別を取得します。
 * @return 選択中のSkillType。
 */
SkillType getCurrentSkillType();

/**
 * @brief 現在選択されているスキル種別を切り替えます（BA <-> BS）。
 * @details 切替先が選択不可（SP0時のBS等）なら選択状態を変更しない。
 */
void toggleCurrentSkillType();

/**
 * @brief 指定したスキル種別が現在選択可能かを返します。
 * @details SAは常時不可。BSはActorの陣営SPが0のとき不可
 *          （Actor未決定時はSP検査をしない）。BAは常時可。
 */
bool isSkillTypeSelectable(SkillType type);

/**
 * @brief タイムラインの初期化および初期Actorの解決を行います。
 */
void initTimeline();

/**
 * @brief 履歴を1ステップ前に巻き戻します（Undo）。
 */
void backupTurn();

/**
 * @brief 履歴を1ステップ進めます（Redo）。
 */
void forwardTurn();

/**
 * @brief 表示ライン上の指定位置の履歴ステップへ復元します。
 * @param index 復元する表示ライン上の位置（0 = 初期状態）。
 */
void restoreHistoryStep(int index);

/**
 * @brief スキル実行およびターゲット選択を伴い、履歴を記録・分岐してターンを進めます。
 * @details 既存の子行動と同じ行動なら再生し、一致しない行動なら新分岐を作成する
 *          （過去の履歴は破棄しない）。
 * @param skillType 実行するスキル種別。
 * @param targetTeam 対象チーム（0 = enemy, 1 = ally）。
 * @param targetSlot 対象スロットインデックス（0～4）。
 */
void proceedTurnWithHistory(SkillType skillType, int targetTeam, int targetSlot);

#endif // SIMULATION_LOGIC_HPP
