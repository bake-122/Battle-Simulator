/**
 * @file formationView.cpp
 * @brief 編成画面UIの実装およびキオク選択・パラメータ編集・ステータス計算処理。
 */

#include <algorithm>
#include <charconv>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iterator>
#include <utility>
#include <unordered_map>

#include "formationView.h"

#include "imgui/imgui.h"
#include "image_loader.hpp"
#include "../simulation/simulationView.hpp"


// ===========================================================
// 定数・ヘルパー (このファイル内でのみ使用)
// ===========================================================

namespace
{
	// -------------------------------------------------------
	// スロットのレイアウト
	//  背景パネルは Ally が画面左側、Enemy が画面右側。
	//  スロットは両陣営とも、パネルの左側に同じ相対位置で配置する。
	// -------------------------------------------------------

	constexpr float SLOT_MARGIN  = 18.0f;  // スロット間の余白
	constexpr float SLOT_SIZE	= 88.0f;  // スロットサイズ
	constexpr float SLOT_START_X = 60.0f;  // 画面端からのスロット位置
	constexpr float SLOT_START_Y = 78.0f;
	constexpr float SLOT_PITCH   = SLOT_SIZE + SLOT_MARGIN;

	// スロット横のユニット情報
	constexpr float INFO_OFFSET_X = 24.0f;
	constexpr float INFO_WIDTH	= 300.0f;

	// キオク選択Window
	constexpr float SELECTOR_WIDTH  = 600.0f;
	constexpr float SELECTOR_HEIGHT = 400.0f;
	constexpr float SELECTOR_MARGIN = 16.0f;

	// スロット円周
	constexpr float CIRCLE_THICKNESS = 2.5f;
	constexpr int   CIRCLE_SEGMENTS  = 64;

	// 背景パネル
	constexpr float PANEL_MARGIN_X = 40.0f;   // ウィンドウ端からの距離
	constexpr float PANEL_MARGIN_Y = 32.0f;   // 最初/最後のスロットからの距離
	constexpr float PANEL_RADIUS   = 10.0f;
	constexpr float ROW_RADIUS	 = PANEL_RADIUS * 2.0f;
	constexpr float ROW_MARGIN_X   = 10.0f;
	constexpr float ROW_MARGIN_Y   = SLOT_MARGIN / 4.0f;

	constexpr float PANEL_WIDTH =
		WINDOW_WIDTH / 2.1f - PANEL_MARGIN_X * 2.0f;

	// 背景パネル左端から見たスロットの相対X位置
	constexpr float SLOT_OFFSET_IN_PANEL =
		SLOT_START_X - PANEL_MARGIN_X;

	// 色 (共通)
	constexpr ImU32 COLOR_CIRCLE_HIGHLIGHT = IM_COL32(80, 220, 100, 255);
	constexpr ImU32 COLOR_CIRCLE_NORMAL	= IM_COL32(200, 200, 200, 255);

	// 色 (Ally: 青系)
	constexpr ImU32 COLOR_ALLY_PANEL = IM_COL32(51, 60, 73, 255);
	constexpr ImU32 COLOR_ALLY_ROW   = IM_COL32(86, 101, 122, 255);

	// 色 (Enemy: 薄い赤系)
	constexpr ImU32 COLOR_ENEMY_PANEL = IM_COL32(122, 62, 70, 255);
	constexpr ImU32 COLOR_ENEMY_ROW   = IM_COL32(168, 100, 108, 255);

	// 両陣営を順に処理するための配列
	constexpr PartySide ALL_SIDES[] = { PartySide::Ally, PartySide::Enemy };


	// -------------------------------------------------------
	// 陣営ごとの値
	// -------------------------------------------------------

	/** @brief 陣営が味方（Ally）であるかを判定します。 */
	bool isAlly(PartySide side)
	{
		return side == PartySide::Ally;
	}

	/** @brief Window名識別用の陣営文字列タグ（"Ally" または "Enemy"）を取得します。 */
	const char* sideTag(PartySide side)
	{
		return isAlly(side) ? "Ally" : "Enemy";
	}

	/** @brief 陣営のパネル背景色を取得します。 */
	ImU32 panelColor(PartySide side)
	{
		return isAlly(side) ? COLOR_ALLY_PANEL : COLOR_ENEMY_PANEL;
	}

	/** @brief 陣営のスロット行背景色を取得します。 */
	ImU32 rowColor(PartySide side)
	{
		return isAlly(side) ? COLOR_ALLY_ROW : COLOR_ENEMY_ROW;
	}

	/**
	 * @brief 背景パネルの左端X座標を計算します。
	 * @param side 対象陣営。
	 * @return 左端X座標。
	 */
	float panelLeft(PartySide side)
	{
		const float panel_start = isAlly(side)
			? PANEL_MARGIN_X
			: WINDOW_WIDTH - PANEL_MARGIN_X - PANEL_WIDTH;

		return panel_start;
	}

	/**
	 * @brief スロットのX座標を計算します。
	 * @param side 対象陣営。
	 * @return スロットX座標。
	 */
	float slotX(PartySide side)
	{
		return panelLeft(side) + SLOT_OFFSET_IN_PANEL;
	}

	/**
	 * @brief 指定スロットの左上座標を取得します。
	 * @param side 対象陣営。
	 * @param slot_index スロットインデックス（0～4）。
	 * @return スロットの左上座標。
	 */
	ImVec2 slotPosition(PartySide side, int slot_index)
	{
		return ImVec2(
			slotX(side),
			SLOT_START_Y + slot_index * SLOT_PITCH
		);
	}
	
	/**
	 * @brief スロット横のユニット情報描画開始座標を取得します。
	 * @param side 対象陣営。
	 * @param slot_index スロットインデックス（0～4）。
	 * @return 情報Windowの左上座標。
	 */
	ImVec2 infoPosition(PartySide side, int slot_index)
	{
		const ImVec2 slot_pos = slotPosition(side, slot_index);

		return ImVec2(
			slot_pos.x + SLOT_SIZE + INFO_OFFSET_X,
			slot_pos.y
		);
	}

	/**
	 * @brief キオク選択Windowの左上座標を計算します。
	 * @param side 対象陣営。
	 * @param slot_index スロットインデックス（0～4）。
	 * @return 選択Windowの左上座標。
	 */
	ImVec2 selectorPosition(PartySide side, int slot_index)
	{
		const ImVec2 slot_pos = slotPosition(side, slot_index);

		float x;

		if (isAlly(side))
		{
			// 味方 → 右側（敵パーティー側）
			x = slot_pos.x + SLOT_SIZE + SELECTOR_MARGIN;
		}
		else
		{
			// 敵 → 左側（味方パーティー側）
			x = slot_pos.x - SELECTOR_WIDTH - SELECTOR_MARGIN;
		}

		// 画面外に出ないようにする
		x = std::clamp(
			x,
			0.0f,
			WINDOW_WIDTH - SELECTOR_WIDTH
		);

		float y = slot_pos.y;

		if (y + SELECTOR_HEIGHT > WINDOW_HEIGHT)
			y = WINDOW_HEIGHT - SELECTOR_HEIGHT;

		y = std::max(y, 0.0f);

		return ImVec2(x, y);
	}

	/**
	 * @brief 整数入力用コールバックデータ構造体。
	 */
	struct IntegerInputCallbackData
	{
		int* value;       /**< 入力対象の整数ポインタ */
		int min_value;    /**< 最小許容値 */
		int max_value;    /**< 最大許容値 */
	};

	/**
	 * @brief ImGuiの整数InputText用コールバック関数。
	 * @details 数字以外の入力をフィルタリングし、空文字処理や先行ゼロの削除、範囲clampを行います。
	 * @param data ImGuiコールバックデータ。
	 * @return 処理結果コード。
	 */
	int integerInputCallback(ImGuiInputTextCallbackData* data)
	{
		auto* callback_data =
			static_cast<IntegerInputCallbackData*>(data->UserData);

		if (!callback_data || !callback_data->value)
			return 0;


		// -------------------------------------------------------
		// 0～9以外は禁止
		// -------------------------------------------------------
		if (data->EventFlag == ImGuiInputTextFlags_CallbackCharFilter)
		{
			if (data->EventChar < '0' || data->EventChar > '9')
				return 1;

			return 0;
		}

		if (data->EventFlag != ImGuiInputTextFlags_CallbackEdit)
			return 0;


		// -------------------------------------------------------
		// 空文字なら 最小値 を挿入
		// -------------------------------------------------------

		if (data->BufTextLen == 0)
		{
			char text[32];

			const int fill_value =
				callback_data->min_value;

			const int length =
				std::snprintf(
					text,
					sizeof(text),
					"%d",
					fill_value
				);

			if (length > 0)
			{
				data->InsertChars(
					0,
					text
				);
			}

			*callback_data->value = fill_value;

			return 0;
		}


		// -------------------------------------------------------
		// 先頭の余分な0を除去。最後の0は残すため、"0"の場合は処理しない。
		// -------------------------------------------------------

		if (data->BufTextLen >= 2 &&
			data->Buf[0] == '0')
		{
			int leading_zero_count = 0;

			while (leading_zero_count < data->BufTextLen - 1 &&
				   data->Buf[leading_zero_count] == '0')
			{
				++leading_zero_count;
			}

			if (leading_zero_count > 0)
			{
				data->DeleteChars(
					0,
					leading_zero_count
				);
			}
		}


		// -------------------------------------------------------
		// 数値へ変換
		// -------------------------------------------------------

		int parsed_value = 0;

		const char* begin = data->Buf;
		const char* end   = data->Buf + data->BufTextLen;

		const auto result =
			std::from_chars(
				begin,
				end,
				parsed_value
			);


		if (result.ec != std::errc{} ||
			result.ptr != end)
		{
			return 0;
		}

		const int clamped_value =
			std::clamp(
				parsed_value,
				callback_data->min_value,
				callback_data->max_value
			);


		*callback_data->value = clamped_value;


		// 範囲内なら文字列変更不要
		if (parsed_value == clamped_value)
			return 0;


		// -------------------------------------------------------
		// 範囲外だった場合 => ImGuiの編集APIを使用して安全に置換する。
		// -------------------------------------------------------

		char text[32];

		const int length =
			std::snprintf(
				text,
				sizeof(text),
				"%d",
				clamped_value
			);

		if (length > 0)
		{
			data->DeleteChars(
				0,
				data->BufTextLen
			);

			data->InsertChars(
				0,
				text
			);
		}

		return 0;
	}


	/**
	 * @brief 文字列バッファに整数値をフォーマットして書き込みます。
	 * @param buffer 出力先固定長バッファ。
	 * @param value 書き込む整数値。
	 */
	void setIntegerBuffer(
		std::array<char, 16>& buffer,
		int value)
	{
		std::snprintf(
			buffer.data(),
			buffer.size(),
			"%d",
			value
		);
	}

	/**
	 * @brief 整数パラメータ入力UI（[-] / 数値Input / [+]）を描画します。
	 * @param label パラメータ名ラベル。
	 * @param buffer 入力用テキストバッファ。
	 * @param value 編集対象の整数参照。
	 * @param min_value 最小許容値。
	 * @param max_value 最大許容値。
	 */
	void drawIntegerParameter(
		const char* label,
		std::array<char, 16>& buffer,
		int& value,
		int min_value,
		int max_value)
	{
		ImGui::PushID(label);

		// -------------------------------------------------------
		// パラメータ名
		// -------------------------------------------------------

		char label_text[128];

		std::snprintf(
			label_text,
			sizeof(label_text),
			"%s [%d-%d]",
			label,
			min_value,
			max_value
		);

		ImGui::TextUnformatted(label_text);


		// -------------------------------------------------------
		// 入力行
		// -------------------------------------------------------

		constexpr float INPUT_X = 0.0f;

		ImGui::SetCursorPosX(INPUT_X);


		// =======================================================
		// - ボタン
		// =======================================================

		static std::unordered_map<ImGuiID, double> minus_repeat_time;

		constexpr double REPEAT_DELAY = 0.35; // 長押し開始まで
		constexpr double REPEAT_RATE  = 0.04; // 連続入力間隔

		ImGui::SmallButton("-");

		const ImGuiID minus_id = ImGui::GetItemID();

		const double current_time = ImGui::GetTime();

		bool minus_pressed = false;

		// 押した瞬間
		if (ImGui::IsItemActivated())
		{
			minus_pressed = true;

			minus_repeat_time[minus_id] =
				current_time + REPEAT_DELAY;
		}

		// 長押し中
		if (ImGui::IsItemActive())
		{
			auto it = minus_repeat_time.find(minus_id);

			if (it != minus_repeat_time.end() &&
				current_time >= it->second)
			{
				minus_pressed = true;

				it->second =
					current_time + REPEAT_RATE;
			}
		}
		else
		{
			// ボタンから離れたらリセット
			minus_repeat_time.erase(minus_id);
		}


		if (minus_pressed)
		{
			value = std::max(
				value - 1,
				min_value
			);

			setIntegerBuffer(
				buffer,
				value
			);
		}


		ImGui::SameLine();


		// =======================================================
		// 数値入力
		// =======================================================

		ImGui::SetNextItemWidth(80.0f);

		IntegerInputCallbackData callback_data
		{
			&value,
			min_value,
			max_value
		};

		ImGui::InputText(
			"##Value",
			buffer.data(),
			buffer.size(),
			ImGuiInputTextFlags_CallbackCharFilter |
			ImGuiInputTextFlags_CallbackEdit,
			integerInputCallback,
			&callback_data
		);

		ImGui::SameLine();


		// =======================================================
		// + ボタン
		// =======================================================

		static std::unordered_map<ImGuiID, double> plus_repeat_time;

		ImGui::SmallButton("+");

		const ImGuiID plus_id = ImGui::GetItemID();

		bool plus_pressed = false;

		// 押した瞬間
		if (ImGui::IsItemActivated())
		{
			plus_pressed = true;

			plus_repeat_time[plus_id] =
				current_time + REPEAT_DELAY;
		}

		// 長押し中
		if (ImGui::IsItemActive())
		{
			auto it = plus_repeat_time.find(plus_id);

			if (it != plus_repeat_time.end() &&
				current_time >= it->second)
			{
				plus_pressed = true;

				it->second =
					current_time + REPEAT_RATE;
			}
		}
		else
		{
			// ボタンから離れたらリセット
			plus_repeat_time.erase(plus_id);
		}


		if (plus_pressed)
		{
			value = std::min(
				value + 1,
				max_value
			);

			setIntegerBuffer(
				buffer,
				value
			);
		}


		ImGui::PopID();
	}


	// -------------------------------------------------------
	// ユーティリティ
	// -------------------------------------------------------

	/**
	 * @brief Kioku_dataの有効なインデックス範囲内であるかを判定します。
	 * @param index 検査するインデックス。
	 * @return 有効範囲内ならtrue。
	 */
	bool isValidKiokuIndex(int index)
	{
		return index >= 0 &&
			   index < static_cast<int>(std::size(Kioku_data));
	}

	/**
	 * @brief キオクの表示名を取得します。
	 * @param unit 対象キオクデータ。
	 * @return 名前文字列（nullptrの場合は "Unknown"）。
	 */
	const char* unitName(const Character& unit)
	{
		return unit.name ? unit.name : "Unknown";
	}

	// -------------------------------------------------------
	// ホバー時のステータステーブル
	// -------------------------------------------------------

	/**
	 * @brief ホバーツールチップのステータステーブル1行分を描画します。
	 * @tparam Stats ステータス構造体型。
	 * @param label レベル表記ラベル。
	 * @param stats ステータス値。
	 */
	template <typename Stats>
	void drawStatsRow(const char* label, const Stats& stats)
	{
		ImGui::TableNextRow();

		ImGui::TableSetColumnIndex(0);
		ImGui::TextUnformatted(label);

		ImGui::TableSetColumnIndex(1);
		ImGui::Text("%d", stats.hp);

		ImGui::TableSetColumnIndex(2);
		ImGui::Text("%d", stats.atk);

		ImGui::TableSetColumnIndex(3);
		ImGui::Text("%d", stats.def);
	}

	/**
	 * @brief ホバーツールチップの各レベル帯ステータステーブルを描画します。
	 * @param ch 対象キオクデータ。
	 */
	void drawStatsTable(const Character& ch)
	{
		constexpr ImGuiTableFlags flags =
			ImGuiTableFlags_Borders |
			ImGuiTableFlags_RowBg;

		if (!ImGui::BeginTable("HoverStatsTable", 4, flags))
			return;

		ImGui::TableSetupColumn("Level");
		ImGui::TableSetupColumn("HP");
		ImGui::TableSetupColumn("ATK");
		ImGui::TableSetupColumn("DEF");
		ImGui::TableHeadersRow();

		drawStatsRow("Lv1",   ch.stats.lv1);
		drawStatsRow("Lv120", ch.stats.lv120);
		drawStatsRow("Lv140", ch.stats.lv140);
		drawStatsRow("Lv160", ch.stats.lv160);

		ImGui::EndTable();
	}


	// -------------------------------------------------------
	// スロット円周
	// -------------------------------------------------------

	/**
	 * @brief スロット外周の円を描画します。
	 * @param draw_list 描画リスト。
	 * @param center 円の中心座標。
	 * @param highlighted ハイライト状態フラグ。
	 */
	void drawSlotCircle(ImDrawList* draw_list,
						const ImVec2& center,
						bool highlighted)
	{
		const float radius =
			SLOT_SIZE * 0.5f - CIRCLE_THICKNESS * 0.5f;

		draw_list->AddCircle(
			center,
			radius,
			highlighted ? COLOR_CIRCLE_HIGHLIGHT
						: COLOR_CIRCLE_NORMAL,
			CIRCLE_SEGMENTS,
			CIRCLE_THICKNESS
		);
	}
}


// ===========================================================
// Constructor / initialize / update / draw
// ===========================================================

FormationView::FormationView()
{
}


void FormationView::initialize()
{
	// Kioku_data の有効なユニット (id != 0) だけを一覧化
	valid_unit_indices.clear();

	for (int i = 0; i < static_cast<int>(std::size(Kioku_data)); ++i)
	{
		if (Kioku_data[i].id != 0)
		{
			valid_unit_indices.push_back(i);
		}
	}
}


void FormationView::update()
{
	// 現在は各UI操作を draw 内で直接処理しているため、
	// 特別な更新処理はない。
	// 将来的に入力処理と描画処理を分離する場合は、ここへ移す。
}


SceneType FormationView::draw()
{
	SceneType next_scene = SceneType::Formation;
	for (const PartySide side : ALL_SIDES)
	{
		drawUnitSlots(side);
		drawUnitInformation(side);
		drawUnitSlotBackground(side);
	}

	// -----------------------------------------------------------
	// Start Simulation ボタン
	// -----------------------------------------------------------

	constexpr float BUTTON_WIDTH  = 128.0f;
	constexpr float BUTTON_HEIGHT = 32.0f;
	constexpr float BUTTON_Y	  = WINDOW_HEIGHT - 80.0f;
	constexpr float BUTTON_ROUNDING = 10.0f;

	const float button_x =
		(WINDOW_WIDTH - BUTTON_WIDTH) * 0.5f;

	const ImVec2 button_min(button_x, BUTTON_Y);
	const ImVec2 button_max(button_x + BUTTON_WIDTH, BUTTON_Y + BUTTON_HEIGHT);

	ImGuiWindowFlags button_window_flags =
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoBackground;

	// キオク選択中は操作を完全に除外
	if (show_unit_selector)
	{
		button_window_flags |= ImGuiWindowFlags_NoInputs;
	}

	ImGui::SetNextWindowPos(button_min);
	ImGui::SetNextWindowSize(ImVec2(BUTTON_WIDTH, BUTTON_HEIGHT));

	ImGui::Begin(
		"StartSimulationButtonWindow",
		nullptr,
		button_window_flags
	);

	ImGuiIO& io = ImGui::GetIO();

	// -------------------------------------------------------
	// 両陣営に1つ以上のユニットがあるかチェック
	// -------------------------------------------------------

	const bool hasAllyUnit = [&]() -> bool
	{
		for (int i = 0; i < SLOT_COUNT; ++i)
		{
			if (ally_slots.unit_indices[i] >= 0) return true;
		}
		return false;
	}();

	const bool hasEnemyUnit = [&]() -> bool
	{
		for (int i = 0; i < SLOT_COUNT; ++i)
		{
			if (enemy_slots.unit_indices[i] >= 0) return true;
		}
		return false;
	}();

	const bool button_disabled =
		!hasAllyUnit || !hasEnemyUnit || show_unit_selector;

	const bool hovered =
		!button_disabled &&
		io.MousePos.x >= button_min.x &&
		io.MousePos.x <= button_max.x &&
		io.MousePos.y >= button_min.y &&
		io.MousePos.y <= button_max.y;

	// -------------------------------------------------------
	// シミュレーション開始処理（クリック / Enterキー共通）
	// -------------------------------------------------------

	auto startSimulation = [&]()
	{
		TeamState state{};
		for (int i = 0; i < SLOT_COUNT; ++i)
		{
			const int ally_data_idx = ally_slots.unit_indices[i];
			if (ally_data_idx >= 0)
			{
				const Character& ch = Kioku_data[ally_data_idx];
				const Stats stats = calcStats(ally_slots.parameters[i], ch);
				state.ally[i].character = ch;
				state.ally[i].hp = stats.hp;
				state.ally[i].atk = stats.atk;
				state.ally[i].def = stats.def;
				state.ally[i].currentMP = 0;
			}

			const int enemy_data_idx = enemy_slots.unit_indices[i];
			if (enemy_data_idx >= 0)
			{
				const Character& ch = Kioku_data[enemy_data_idx];
				const Stats stats = calcStats(enemy_slots.parameters[i], ch);
				state.enemy[i].character = ch;
				state.enemy[i].hp = stats.hp;
				state.enemy[i].atk = stats.atk;
				state.enemy[i].def = stats.def;
				state.enemy[i].currentMP = 0;
			}
		}

		// AV は simulationLogic 側 initTimeline() 処理する。

		setSimulationTeamState(state);
		next_scene = SceneType::Simulation;
	};

	if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
	{
		startSimulation();
	}

	// Enter キーでシミュレーション開始
	if (!button_disabled &&
		(ImGui::IsKeyPressed(ImGuiKey_Enter) ||
		 ImGui::IsKeyPressed(ImGuiKey_KeypadEnter)))
	{
		startSimulation();
	}

	// -----------------------------------------------------------
	// ボタン描画
	// -----------------------------------------------------------

	ImDrawList* draw_list = ImGui::GetWindowDrawList();

	ImU32 button_color;
	if (button_disabled)
	{
		button_color = IM_COL32(150, 150, 150, 255);
	}
	else if (hovered)
	{
		button_color = IM_COL32(185, 230, 185, 255);
	}
	else
	{
		button_color = IM_COL32(170, 220, 170, 255);
	}

	draw_list->AddRectFilled(
		button_min,
		button_max,
		button_color,
		BUTTON_ROUNDING
	);

	// テキスト
	const char* text = "Start Simulation";

	const ImVec2 text_size =
		ImGui::CalcTextSize(text);

	const ImVec2 text_pos(
		button_x + (BUTTON_WIDTH  - text_size.x) * 0.5f,
		BUTTON_Y + (BUTTON_HEIGHT - text_size.y) * 0.5f
	);

	draw_list->AddText(
		text_pos,
		IM_COL32(40, 40, 40, 255),
		text
	);

	ImGui::End();

	// 選択Windowは陣営共通 (1つだけ) - 最前面に表示するため最後に呼び出す
	drawUnitSelector();

	return next_scene;
}

void FormationView::initializeParameterInput(
	PartySide side,
	int slot_index,
	int data_index,
	const UnitParameters& params)
{
	if (parameter_input.initialized &&
		parameter_input.side == side &&
		parameter_input.slot_index == slot_index &&
		parameter_input.data_index == data_index)
	{
		return;
	}

	std::snprintf(
		parameter_input.buffers[0].data(),
		parameter_input.buffers[0].size(),
		"%d",
		params.ascension
	);

	std::snprintf(
		parameter_input.buffers[1].data(),
		parameter_input.buffers[1].size(),
		"%d",
		params.level
	);

	std::snprintf(
		parameter_input.buffers[2].data(),
		parameter_input.buffers[2].size(),
		"%d",
		params.heartphial
	);

	std::snprintf(
		parameter_input.buffers[3].data(),
		parameter_input.buffers[3].size(),
		"%d",
		params.magicLevel
	);

	std::snprintf(
		parameter_input.buffers[4].data(),
		parameter_input.buffers[4].size(),
		"%d",
		params.specialLvl
	);

	parameter_input.side	   = side;
	parameter_input.slot_index = slot_index;
	parameter_input.data_index = data_index;
	parameter_input.initialized = true;
}


// ===========================================================
// パーティー操作
// ===========================================================

FormationView::PartySlots& FormationView::getSlots(PartySide side)
{
	return isAlly(side) ? ally_slots : enemy_slots;
}


// ユニットをスロットへ割り当てる。
// 同一判定は同じパーティー内のみ。
//  - 同じパーティーの別スロットに既にいる → ユニットとパラメータを交換
//  - 他方のパーティーにいる			   → 影響なし (そのまま選択)
void FormationView::assignUnitToSlot(PartySide side,
									 int slot_index,
									 int data_index)
{
	PartySlots& slots = getSlots(side);

	const auto begin = slots.unit_indices.begin();
	const auto end   = slots.unit_indices.end();
	const auto found = std::find(begin, end, data_index);

	const int other_slot_index =
		(found != end)
			? static_cast<int>(found - begin)
			: -1;

	if (other_slot_index >= 0 && other_slot_index != slot_index)
	{
		std::swap(
			slots.unit_indices[other_slot_index],
			slots.unit_indices[slot_index]
		);
		std::swap(
			slots.parameters[other_slot_index],
			slots.parameters[slot_index]
		);
	}
	else
	{
		slots.unit_indices[slot_index] = data_index;
	}
}


// スロットからユニットを削除
void FormationView::clearUnitFromSlot(
	PartySide side,
	int slot_index
)
{
	if (slot_index < 0 ||
		slot_index >= SLOT_COUNT)
	{
		return;
	}

	PartySlots& slots = getSlots(side);

	// ユニットを外す
	slots.unit_indices[slot_index] = -1;

	// パラメータを初期値へ戻す
	slots.parameters[slot_index] = UnitParameters{};

	// 入力バッファを次回選択時に再初期化させる
	if (parameter_input.side == side &&
		parameter_input.slot_index == slot_index)
	{
		parameter_input.initialized = false;
	}
}


// ===========================================================
// ゲームルール・表示用変換
// ===========================================================

// 限界突破段階ごとの必殺技レベル上限
int FormationView::getSpecialLevelMax(int ascension) const {
	if (ascension <= 2) return 4;
	if (ascension <= 4) return 7;
	return 10;
}

int FormationView::getMagicLevelMax(int level) const {
	if (level <= 29) return 30;
	if (level <= 59) return 60;
	if (level <= 89) return 90;
	if (level <= 100) return 120;
	if (level <= 109) return 125;
	if (level <= 135) return 130;
	if (level <= 145) return 135;
	return 140;
}


const char* FormationView::elementToString(Element elem) const
{
	switch (elem)
	{
		case Element::Flame:  return "Flame";
		case Element::Forest: return "Forest";
		case Element::Aqua:   return "Aqua";
		case Element::Light:  return "Light";
		case Element::Dark:   return "Dark";
		case Element::Void:   return "Void";
		default:			  return "None";
	}
}


const char* FormationView::roleToString(Role role) const
{
	switch (role)
	{
		case Role::Attacker: return "Attacker";
		case Role::Breaker:  return "Breaker";
		case Role::Healer:   return "Healer";
		case Role::Buffer:   return "Buffer";
		case Role::Debuffer: return "Debuffer";
		case Role::Defender: return "Defender";
		default:			 return "None";
	}
}

Stats FormationView::calcMagicLevelStats(const UnitParameters& params, const Character& unit) const
{
	const BonusStats* bonusStats = nullptr;
	int bonusNum;
	if (unit.rarity == 5)
	{
		bonusStats = MAGICLEVEL_STATS_HIGH;
		bonusNum = static_cast<int>(std::size(MAGICLEVEL_STATS_HIGH));
	}
	else
	{
		bonusStats = MAGICLEVEL_STATS_LOW;
		bonusNum = static_cast<int>(std::size(MAGICLEVEL_STATS_LOW));
	}
	
	Stats result{ 0, 0, 0 };
	
	for (int i = 0; i < bonusNum; ++i)
	{
		if(params.magicLevel < bonusStats[i].level) break;
		
		result += bonusStats[i].bonus;
	}
	
	return result;
}

Stats FormationView::calcHeartphialStats(const UnitParameters& params, const Character& unit) const
{
	const BonusStats* bonusStats = nullptr;
	int bonusNum;
	if (unit.id == 27001 || unit.id == 29001 || unit.id == 30001 || unit.id == 38001)
	{
		bonusStats = HEARTPHIAL_STATS_LOW;
		bonusNum = static_cast<int>(std::size(HEARTPHIAL_STATS_LOW));
	}
	else
	{
		bonusStats = HEARTPHIAL_STATS_HIGH;
		bonusNum = static_cast<int>(std::size(HEARTPHIAL_STATS_HIGH));
	}
	
	Stats result{ 0, 0, 0 };
	
	for (int i = 0; i < bonusNum; ++i)
	{
		if(params.heartphial < bonusStats[i].level) break;
		
		result += bonusStats[i].bonus;
	}
	
	return result;
}

Stats FormationView::calcStats(const UnitParameters& params, const Character& unit) const
{
	// 限界突破による倍率
	constexpr int ASCENSION_RATIO[] = { 100, 102, 104, 106, 108, 110 };
	
	struct Breakpoint
	{
		int			 level;
		const Stats* stats;
	};
	
	const Breakpoint breakpoints[] = 
	{
		{ 1, &unit.stats.lv1 },
		{ 120, &unit.stats.lv120 },
		{ 140, &unit.stats.lv140 },
		{ 160, &unit.stats.lv160 },
	};
	
	// levelが属するBreakpointの区間を探す
	int index = 0;
	while (index < static_cast<int>(std::size(breakpoints)) - 2 && params.level > breakpoints[index + 1].level)
		++index;
	
	const Breakpoint& low = breakpoints[index];
	const Breakpoint& high = breakpoints[index + 1];
	
	// Level, Ascensionによる補正
	auto calc = [&](int lStat, int hStat)
	{
		int baseVal = lStat + (hStat - lStat) * (params.level - low.level) / (high.level - low.level);
		return baseVal * ASCENSION_RATIO[params.ascension] / 100;
	};
	
	return Stats{
		calc(low.stats->hp, high.stats->hp),
		calc(low.stats->atk, high.stats->atk),
		calc(low.stats->def, high.stats->def)
	} + calcMagicLevelStats(params, unit) + calcHeartphialStats(params, unit);
}
	


// ===========================================================
// ユニットスロット
// ===========================================================

void FormationView::drawUnitSlots(PartySide side)
{
	PartySlots& slots = getSlots(side);

	const ImVec2 slot_size(SLOT_SIZE, SLOT_SIZE);
	const float  slot_radius = SLOT_SIZE * 0.5f;

	for (int slot_index = 0; slot_index < SLOT_COUNT; ++slot_index)
	{
		const ImVec2 slot_pos = slotPosition(side, slot_index);
		const ImVec2 slot_center(
			slot_pos.x + slot_radius,
			slot_pos.y + slot_radius
		);

		// 現在のスロットに入っているユニット
		const int  selected_index = slots.unit_indices[slot_index];
		const bool has_unit	   = isValidKiokuIndex(selected_index);

		// マウスが円の内側にあるか
		const ImVec2 mouse_pos = ImGui::GetMousePos();
		const float  dx = mouse_pos.x - slot_center.x;
		const float  dy = mouse_pos.y - slot_center.y;
		const bool inside_circle =
			(dx * dx + dy * dy) <= (slot_radius * slot_radius);

		// 円周の強調状態
		//  - 選択Window表示中: 選択中の陣営・スロットのみ
		//  - それ以外		: マウスが乗っているスロット
		const bool slot_highlighted =
			show_unit_selector
				? (selector_side == side &&
				   selector_slot_index == slot_index)
				: inside_circle;

		// 円周は「強調中」または「空スロット」のときに描画
		const bool draw_circle = slot_highlighted || !has_unit;


		// ---------------------------------------------------
		// スロットWindow
		// ---------------------------------------------------

		char window_name[64];
		std::snprintf(
			window_name, sizeof(window_name),
			"Unit Slot %d##%sSlot%d",
			slot_index + 1, sideTag(side), slot_index
		);

		ImGuiWindowFlags window_flags =
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoScrollbar |
			ImGuiWindowFlags_NoBackground;

		// キオク選択中は (Ally/Enemy問わず) 入力から完全に除外
		if (show_unit_selector)
		{
			window_flags |= ImGuiWindowFlags_NoInputs;
		}

		ImGui::SetNextWindowPos(slot_pos);
		ImGui::SetNextWindowSize(slot_size);
		ImGui::Begin(window_name, nullptr, window_flags);


		// クリックでキオク選択Windowを開く
		if (!show_unit_selector)
		{
			ImGui::InvisibleButton("UnitSlotButton", slot_size);

			if (inside_circle &&
				ImGui::IsItemClicked(ImGuiMouseButton_Left))
			{
				selector_side	   = side;
				selector_slot_index = slot_index;
				show_unit_selector  = true;
			}
		}

		// ---------------------------------------------------
		// キオク画像 + 属性・ロール画像
		// ---------------------------------------------------
		if (has_unit)
		{
			const Character& unit = Kioku_data[selected_index];
			ImageManager& imageManager = ImageManager::GetInstance();

			// ---------------------------------------------------
			// キオク画像
			// ---------------------------------------------------
			const GLuint unit_texture =
				imageManager.GetTexture(unit.id);

			if (unit_texture != 0)
			{
				ImGui::SetCursorPos(ImVec2(0.0f, 0.0f));

				ImGui::Image(
					(void*)(intptr_t)unit_texture,
					slot_size
				);
			}
		}
		
		// 円周
		if (draw_circle)
		{
			drawSlotCircle(
				ImGui::GetWindowDrawList(),
				slot_center,
				slot_highlighted
			);
		}
		
		if (has_unit)
		{
			const Character& unit = Kioku_data[selected_index];
			ImageManager& imageManager = ImageManager::GetInstance();
			
			// ---------------------------------------------------
			// 属性・ロール画像のサイズ
			// ---------------------------------------------------
			const float sub_size = SLOT_SIZE / 3.0f;

			const ImVec2 sub_icon_size(
				sub_size,
				sub_size
			);


			// ---------------------------------------------------
			// 属性・ロールの中心座標
			// ---------------------------------------------------

			// 左下 1/9 領域の中心
			const ImVec2 element_center(
				slot_pos.x + SLOT_SIZE / 6.0f,
				slot_pos.y + SLOT_SIZE * 5.0f / 6.0f
			);

			// 右下 1/9 領域の中心
			const ImVec2 role_center(
				slot_pos.x + SLOT_SIZE * 5.0f / 6.0f,
				slot_pos.y + SLOT_SIZE * 5.0f / 6.0f
			);


			// ---------------------------------------------------
			// 描画リスト
			// ---------------------------------------------------
			ImDrawList* draw_list = ImGui::GetWindowDrawList();


			// ---------------------------------------------------
			// 黒円 (Elementの背景)
			// ---------------------------------------------------

			draw_list->AddCircleFilled(
				element_center,
				sub_size * 0.475f, // 半径
				IM_COL32(60, 60, 60, 255)
			);


			// ---------------------------------------------------
			// Element画像
			// ---------------------------------------------------
			const GLuint element_texture =
				imageManager.GetElementTexture(unit.element);

			if (element_texture != 0)
			{
				const ImVec2 min_pos(
					element_center.x - sub_size * 0.5f,
					element_center.y - sub_size * 0.5f
				);

				const ImVec2 max_pos(
					element_center.x + sub_size * 0.5f,
					element_center.y + sub_size * 0.5f
				);

				draw_list->AddImage(
					(ImTextureID)(intptr_t)element_texture,
					min_pos,
					max_pos
				);
			}


			// ---------------------------------------------------
			// Role画像
			// ---------------------------------------------------
			const GLuint role_texture =
				imageManager.GetRoleTexture(unit.role);

			if (role_texture != 0)
			{
				const ImVec2 min_pos(
					role_center.x - sub_size * 0.5f,
					role_center.y - sub_size * 0.5f
				);

				const ImVec2 max_pos(
					role_center.x + sub_size * 0.5f,
					role_center.y + sub_size * 0.5f
				);

				draw_list->AddImage(
					(ImTextureID)(intptr_t)role_texture,
					min_pos,
					max_pos
				);
			}
		}

		// マウスホバー時のツールチップ
		if (!show_unit_selector && inside_circle && has_unit)
		{
			drawUnitTooltip(Kioku_data[selected_index]);
		}

		ImGui::End();
	}
}


// ===========================================================
// スロット横のユニット情報
// ===========================================================

void FormationView::drawUnitInformation(PartySide side)
{
	constexpr ImGuiWindowFlags window_flags =
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoBackground |
		ImGuiWindowFlags_NoBringToFrontOnFocus |
		ImGuiWindowFlags_NoFocusOnAppearing;

	PartySlots& slots = getSlots(side);

	for (int slot_index = 0;
		 slot_index < SLOT_COUNT;
		 ++slot_index)
	{
		const int selected_index =
			slots.unit_indices[slot_index];

		// 空スロットは表示しない
		if (!isValidKiokuIndex(selected_index))
			continue;

		const Character& unit =
			Kioku_data[selected_index];

		const UnitParameters& params =
			slots.parameters[slot_index];

		char window_name[64];

		std::snprintf(
			window_name,
			sizeof(window_name),
			"Unit Info %d##%sInfo%d",
			slot_index + 1,
			sideTag(side),
			slot_index
		);

		ImGui::SetNextWindowPos(
			infoPosition(side, slot_index)
		);

		ImGui::SetNextWindowSize(
			ImVec2(INFO_WIDTH, SLOT_SIZE)
		);

		ImGui::Begin(
			window_name,
			nullptr,
			window_flags
		);

		// ---------------------------------------------------
		// 情報部分の行間を狭くする
		// ---------------------------------------------------

		ImGui::PushStyleVar(
			ImGuiStyleVar_ItemSpacing,
			ImVec2(8.0f, 0.0f)
		);

		// ---------------------------------------------------
		// 1行目: ユニット名
		// ---------------------------------------------------

		ImGui::PushFont(
			nullptr,
			ImGui::GetFontSize() * 1.25f
		);

		ImGui::Text(
			"%s",
			unitName(unit)
		);

		ImGui::PopFont();

		// ---------------------------------------------------
		// 2行目
		// ---------------------------------------------------

		ImGui::Text(
			"限界突破: %d, 必殺Lv: %d, HP: %d",
			params.ascension,
			params.specialLvl,
			calcStats(params, unit).hp
		);

		// ---------------------------------------------------
		// 3行目
		// ---------------------------------------------------

		if (unit.id == 1005 || unit.id == 2008)
		{
			ImGui::Text(
				"ATK: %d, DEF: %d, SPD: %d, BP: %d",
				calcStats(params, unit).atk,
				calcStats(params, unit).def,
				unit.spd,
				std::find_if(
					BP_Kioku_List.begin(),
					BP_Kioku_List.end(),
					[&](const auto& p) { return p.first == unit.id; }
				)->second
			);
		}
		else
		{
			ImGui::Text(
				"ATK: %d, DEF: %d, SPD: %d, MP: %d",
				calcStats(params, unit).atk,
				calcStats(params, unit).def,
				unit.spd,
				unit.mp
			);
		}
		
		ImGui::PopStyleVar();

		// ---------------------------------------------------
		// 4行目: Clear
		// ---------------------------------------------------

		ImGui::Spacing();

		if (ImGui::SmallButton("Clear"))
		{
			clearUnitFromSlot(
				side,
				slot_index
			);
		}

		ImGui::End();
	}
}


// ===========================================================
// ホバー時のツールチップ
// ===========================================================

void FormationView::drawUnitTooltip(const Character& ch) const
{
	ImGui::BeginTooltip();

	ImGui::Text("Name: %s",	unitName(ch));
	ImGui::Text("Element: %s", elementToString(ch.element));
	ImGui::Text("Role: %s",	roleToString(ch.role));
	ImGui::Text("SPD: %d  |  MP: %d", ch.spd, ch.mp);

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Text("--- Stats ---");

	drawStatsTable(ch);

	ImGui::EndTooltip();
}


// ===========================================================
// ユニット選択Window (Ally/Enemy共通)
// ===========================================================

void FormationView::drawUnitSelector()
{
	if (!show_unit_selector)
		return;


	// ---------------------------------------------------
	// スロットが有効か
	// ---------------------------------------------------

	const bool slot_valid =
		selector_slot_index >= 0 &&
		selector_slot_index < SLOT_COUNT;


	// ---------------------------------------------------
	// 選択Windowを閉じる共通処理
	// ---------------------------------------------------

	auto close_selector = [this]()
	{
		show_unit_selector  = false;
	};


	// ---------------------------------------------------
	// 編集対象のパーティー
	// ---------------------------------------------------

	PartySlots& slots =
		getSlots(selector_side);


	// ---------------------------------------------------
	// Window
	// ---------------------------------------------------

	ImGui::SetNextWindowPos(
		selectorPosition(
			selector_side,
			selector_slot_index
		),
		ImGuiCond_Appearing
	);

	ImGui::SetNextWindowSize(
		ImVec2(
			SELECTOR_WIDTH,
			SELECTOR_HEIGHT
		),
		ImGuiCond_Always
	);


	ImGui::Begin(
		"キオク選択",
		&show_unit_selector,
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoScrollWithMouse
	);
	
	// ---------------------------------------------------
	// 選択Window外をクリックしたら閉じる
	// ---------------------------------------------------

	if (!ImGui::IsWindowAppearing() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
	{
		const ImVec2 window_pos  = ImGui::GetWindowPos();
		const ImVec2 window_size = ImGui::GetWindowSize();
		const ImVec2 mouse_pos   = ImGui::GetMousePos();

		const bool inside =
			mouse_pos.x >= window_pos.x &&
			mouse_pos.x <= window_pos.x + window_size.x &&
			mouse_pos.y >= window_pos.y &&
			mouse_pos.y <= window_pos.y + window_size.y;

		if (!inside)
		{
			close_selector();

			ImGui::End();

			selector_slot_index = -1;

			return;
		}
	}


	// ---------------------------------------------------
	// 左右2分割
	// ---------------------------------------------------

	ImGui::Columns(
		2,
		"UnitSelectorColumns",
		true
	);


	// ===================================================
	// 左側: ユニット一覧
	// ===================================================

	ImGui::BeginChild(
		"UnitList",
		ImVec2(0, 0),
		false,
		ImGuiWindowFlags_AlwaysVerticalScrollbar
	);

	// ---------------------------------------------------
	// 有効なユニットだけ表示
	// ---------------------------------------------------

	for (const int data_index : valid_unit_indices)
	{
		if (!ImGui::Selectable(
				unitName(Kioku_data[data_index])))
		{
			continue;
		}


		if (!slot_valid)
			continue;


		assignUnitToSlot(
			selector_side,
			selector_slot_index,
			data_index
		);
	}


	ImGui::EndChild();


	// ===================================================
	// 右側: パラメータ編集
	// ===================================================

	ImGui::NextColumn();


	ImGui::BeginChild(
		"ParameterEditor",
		ImVec2(0, 0),
		false,
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoScrollWithMouse
	);


	// ---------------------------------------------------
	// 現在選択されているユニット
	// ---------------------------------------------------

	const int edit_data_index =
		slot_valid
			? slots.unit_indices[selector_slot_index]
			: -1;


	// ---------------------------------------------------
	// 有効なユニットが選択されている場合
	// ---------------------------------------------------

	if (slot_valid &&
		isValidKiokuIndex(edit_data_index))
	{
		const Character& unit =
			Kioku_data[edit_data_index];

		UnitParameters& params =
			slots.parameters[selector_slot_index];


		// ---------------------------------------------------
		// 入力バッファを対象ユニットに同期
		//
		// ユニットを変更したときだけ初期化される。
		// 毎フレーム値を上書きするわけではない。
		// ---------------------------------------------------

		initializeParameterInput(
			selector_side,
			selector_slot_index,
			edit_data_index,
			params
		);


		// ---------------------------------------------------
		// ユニット名
		// ---------------------------------------------------

		ImGui::Text(
			"%s",
			unitName(unit)
		);

		ImGui::Separator();


		// ---------------------------------------------------
		// 必殺レベル上限
		// ---------------------------------------------------

		const int max_special_level =
			getSpecialLevelMax(
				params.ascension
			);


		// Ascension変更によって
		// Special Lvlの上限が下がった場合に対応する。
		const int old_special_level =
			params.specialLvl;


		params.specialLvl =
			std::clamp(
				params.specialLvl,
				1,
				max_special_level
			);


		if (params.specialLvl != old_special_level)
		{
			setIntegerBuffer(
				parameter_input.buffers[4],
				params.specialLvl
			);
		}
		
		// ---------------------------------------------------
		// 魔力解放レベル上限
		// ---------------------------------------------------

		const int max_magic_level =
			getMagicLevelMax(
				params.level
			);


		// Lvl変更によって
		// Magic Lvlの上限が下がった場合に対応する。
		const int old_magic_level =
			params.magicLevel;


		params.magicLevel =
			std::clamp(
				params.magicLevel,
				0,
				max_magic_level
			);


		if (params.magicLevel != old_magic_level)
		{
			setIntegerBuffer(
				parameter_input.buffers[3],
				params.magicLevel
			);
		}


		// ===================================================
		// パラメータ入力
		// ===================================================


		// ---------------------------------------------------
		// 限界突破
		// ---------------------------------------------------

		drawIntegerParameter(
			"限界突破 (Ascension)",
			parameter_input.buffers[0],
			params.ascension,
			0,
			5
		);


		ImGui::Spacing();


		// ---------------------------------------------------
		// レベル
		// ---------------------------------------------------

		drawIntegerParameter(
			"レベル (Lvl)",
			parameter_input.buffers[1],
			params.level,
			1,
			MAX_LEVEL
		);


		ImGui::Spacing();


		// ---------------------------------------------------
		// こころの器
		// ---------------------------------------------------

		drawIntegerParameter(
			"こころの器 (Heartphial)",
			parameter_input.buffers[2],
			params.heartphial,
			1,
			50
		);


		ImGui::Spacing();


		// ---------------------------------------------------
		// 魔力解放
		// ---------------------------------------------------

		drawIntegerParameter(
			"魔力解放 (Magic Lvl)",
			parameter_input.buffers[3],
			params.magicLevel,
			0,
			max_magic_level
		);


		ImGui::Spacing();


		// ---------------------------------------------------
		// 必殺レベル: Ascensionによって上限が変化する。
		// ---------------------------------------------------

		drawIntegerParameter(
			"必殺レベル (Special Lvl)",
			parameter_input.buffers[4],
			params.specialLvl,
			1,
			max_special_level
		);


		ImGui::Spacing();
		ImGui::Spacing();

		if (ImGui::Button("決定"))
		{
			close_selector();
		}
	}
	else
	{
		ImGui::TextDisabled(
			"ユニットが選択されていません"
		);
	}


	ImGui::EndChild();

	ImGui::Columns(1);
	ImGui::End();
	
	if (!show_unit_selector)
	{
		selector_slot_index = -1;
	}
}


// ===========================================================
// 背景矩形 (Ally: 左側 / Enemy: 右側)
// ===========================================================

void FormationView::drawUnitSlotBackground(PartySide side)
{
	// 全スロットを囲む高さ
	constexpr float SLOTS_HEIGHT =
		SLOT_PITCH * (SLOT_COUNT - 1) + SLOT_SIZE;

	constexpr float PANEL_TOP	= SLOT_START_Y - PANEL_MARGIN_Y;
	constexpr float PANEL_HEIGHT = SLOTS_HEIGHT + PANEL_MARGIN_Y * 2.0f;

	const float panel_left = panelLeft(side);

	ImDrawList* draw_list = ImGui::GetBackgroundDrawList();

	// 全体パネル
	draw_list->AddRectFilled(
		ImVec2(panel_left,			   PANEL_TOP),
		ImVec2(panel_left + PANEL_WIDTH, PANEL_TOP + PANEL_HEIGHT),
		panelColor(side),
		PANEL_RADIUS
	);

	// スロットごとの行
	const float row_left  = panel_left + ROW_MARGIN_X;
	const float row_width = PANEL_WIDTH - ROW_MARGIN_X * 2.0f;

	constexpr float ROW_HEIGHT = SLOT_SIZE + ROW_MARGIN_Y * 2.0f;

	for (int i = 0; i < SLOT_COUNT; ++i)
	{
		const float row_top = slotPosition(side, i).y - ROW_MARGIN_Y;

		draw_list->AddRectFilled(
			ImVec2(row_left,			 row_top),
			ImVec2(row_left + row_width, row_top + ROW_HEIGHT),
			rowColor(side),
			ROW_RADIUS
		);
	}
}