/**
 * @file simulationView.cpp
 * @brief 戦闘シミュレーション画面のUI描画・入力処理の実装。
 */

#include "simulationView.hpp"
#include "../imgui/imgui.h"
#include "../image_loader.hpp"
#include "../window_config.h"

#include <algorithm>
#include <vector>
#include <string>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <cfloat>


// ===========================================================
// レイアウト
// ===========================================================

/** @brief 左右分割線の既定X座標。 */
constexpr float DEFAULT_SPLIT_X = 840.0f;
/** @brief 上下分割線の既定Y座標。 */
constexpr float DEFAULT_SPLIT_Y = 500.0f;

/** @brief 分割線移動時の最小領域サイズ。 */
constexpr float MIN_DOMAIN = 64.0f;
/** @brief 左右分割線の最大X座標。 */
constexpr float MAX_DOMAIN_X =
    static_cast<float>(WINDOW_WIDTH) - MIN_DOMAIN;
/** @brief 上下分割線の最大Y座標。 */
constexpr float MAX_DOMAIN_Y =
    static_cast<float>(WINDOW_HEIGHT) - MIN_DOMAIN;

/** @brief domain3（アクションパネル）の横幅。 */
constexpr float DOMAIN3_WIDTH = 1080.0f;
/** @brief domain4（Back to Formationボタン等）の横幅。 */
constexpr float DOMAIN4_WIDTH = static_cast<float>(WINDOW_WIDTH) - DOMAIN3_WIDTH;

/** @brief 分割境界線の太さ。 */
constexpr float SPLITTER_SIZE = 4.0f;


// ===========================================================
// Back to Formation ボタン
// ===========================================================

constexpr float BACK_BUTTON_WIDTH    = 180.0f;
constexpr float BACK_BUTTON_HEIGHT   = 32.0f;
constexpr float BACK_BUTTON_MARGIN   = 10.0f;
constexpr float BACK_BUTTON_ROUNDING = 10.0f;


// ===========================================================
// 境界線カラー
// ===========================================================

// 操作可能
constexpr ImU32 ACTIVE_SPLITTER_COLOR =
    IM_COL32(150, 150, 150, 255);

constexpr ImU32 ACTIVE_SPLITTER_HOVER_COLOR =
    IM_COL32(210, 210, 210, 255);

// 操作不能
constexpr ImU32 INACTIVE_SPLITTER_COLOR =
    IM_COL32(70, 70, 70, 255);


// ===========================================================
// ボタンカラー
// ===========================================================

constexpr ImU32 BACK_BUTTON_COLOR =
    IM_COL32(210, 145, 145, 255);

constexpr ImU32 BACK_BUTTON_HOVER_COLOR =
    IM_COL32(230, 165, 165, 255);

constexpr ImU32 BACK_BUTTON_TEXT_COLOR =
    IM_COL32(60, 40, 40, 255);


// ===========================================================
// 分割位置
// ===========================================================

// domain1 / domain2 の境界位置
static float split_x = DEFAULT_SPLIT_X;

// domain1,2 / domain3,4 の境界位置
static float split_y = DEFAULT_SPLIT_Y;

// ドラッグ状態
static bool dragging_vertical   = false;
static bool dragging_horizontal = false;


// ===========================================================
// UI側のスクロール状態
// ===========================================================

static float g_actionLogScrollY = 0.0f;
static int g_lastAutoScrollHistoryIndex = -1;

/**
 * @brief UI側の初期化ラッパー関数。
 */
void setSimulationTeamState(const TeamState& state)
{
    setSimulationTeamStateLogic(state);
    g_actionLogScrollY = 0.0f;
    g_lastAutoScrollHistoryIndex = -1;
}


// ===========================================================
// Simulation Scene
// ===========================================================

/**
 * @brief 戦闘シミュレーション画面を描画し、フレーム更新を行います。
 */
SceneType renderSimulationScene()
{
    SceneType next_scene = SceneType::Simulation;


    // =======================================================
    // SimulationView 全体
    // =======================================================

    // 背景色は formationView と同一 (glClearColor 0.15, 0.15, 0.15 ≈ IM_COL32(38,38,38))
    constexpr ImU32 SIM_BACKGROUND_COLOR = IM_COL32(38, 38, 38, 255);

    ImGui::SetNextWindowPos(
        ImVec2(0.0f, 0.0f),
        ImGuiCond_Always
    );

    ImGui::SetNextWindowSize(
        ImVec2(
            static_cast<float>(WINDOW_WIDTH),
            static_cast<float>(WINDOW_HEIGHT)
        ),
        ImGuiCond_Always
    );

    ImGui::PushStyleVar(
        ImGuiStyleVar_WindowPadding,
        ImVec2(0.0f, 0.0f)
    );

    ImGui::Begin(
        "SimulationView",
        nullptr,
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse
    );

    // Tab キーで BA / BS を切り替える（キーリピートによる連続切り替えは行わない）。
    // SkillType パネルをクリックした場合と同じ切替関数を使い、SA は選択対象にしない。
    if (ImGui::IsKeyPressed(ImGuiKey_Tab, false))
    {
        toggleCurrentSkillType();
    }

    // 背景矩形を描画（domain1-4 の NoBackground を上書きする）
    {
        ImDrawList* bg_draw_list = ImGui::GetWindowDrawList();
        bg_draw_list->AddRectFilled(
            ImVec2(0.0f, 0.0f),
            ImVec2(static_cast<float>(WINDOW_WIDTH), static_cast<float>(WINDOW_HEIGHT)),
            SIM_BACKGROUND_COLOR
        );
    }


    // =======================================================
    // domain1
    // =======================================================

    ImGui::SetCursorPos(
        ImVec2(0.0f, 0.0f)
    );

    ImGui::BeginChild(
        "domain1",
        ImVec2(
            split_x,
            split_y
        ),
        true,
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse |
        ImGuiWindowFlags_NoBackground
    );

    // -------------------------------------------------------
    // av_panel: domain1 左側の角丸白矩形（明度を下げたグレー）
    // -------------------------------------------------------

    {
        constexpr float AV_PANEL_WIDTH   = 160.0f;
        constexpr float AV_PANEL_MARGIN  = 12.0f;
        constexpr float AV_PANEL_ROUNDING = 8.0f;

        // av_panel の位置は domain1 の top/bottom/left から固定マージン
        const ImVec2 child_screen_pos = ImGui::GetCursorScreenPos();

        const ImVec2 panel_min(
            child_screen_pos.x + AV_PANEL_MARGIN,
            child_screen_pos.y + AV_PANEL_MARGIN
        );
        const ImVec2 panel_max(
            child_screen_pos.x + AV_PANEL_MARGIN + AV_PANEL_WIDTH,
            child_screen_pos.y + DEFAULT_SPLIT_Y - AV_PANEL_MARGIN
        );

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        draw_list->AddRectFilled(
            panel_min,
            panel_max,
            IM_COL32(245, 245, 245, 255),
            AV_PANEL_ROUNDING
        );

        // "Timeline" テキスト: av_panel 上側中央寄せ、太字、黒色
        {
            constexpr float TIMELINE_MARGIN_TOP = 16.0f;
            
            const char* timeline_text = "Timeline";
            
            // PushFont してから CalcTextSize（この ImGui バージョンはフォント引数なし）
            ImGui::PushFont(g_font_bold);
            ImVec2 text_size = ImGui::CalcTextSize(timeline_text);
            ImGui::PopFont();
            
            const float text_x = panel_min.x + (AV_PANEL_WIDTH - text_size.x) * 0.5f;
            const float text_y = panel_min.y + TIMELINE_MARGIN_TOP;

            ImGui::PushFont(g_font_bold);
            ImGui::SetCursorScreenPos(ImVec2(text_x, text_y));
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 0, 0, 255));
            ImGui::TextUnformatted(timeline_text);
            ImGui::PopStyleColor();
            ImGui::PopFont();
        }

        // 累積時刻 t: "Timeline" テキストの下に描画、数値の x 座標は AV テキストと揃える
        {
            constexpr float ACCUM_TIME_MARGIN_TOP = 40.0f;
            // text_x = icon_x + TIMELINE_ICON_SIZE + 6.0f = (panel_min.x + 4.0f) + 32.0f + 6.0f = panel_min.x + 42.0f
            constexpr float ACCUM_TIME_VALUE_X_OFFSET = 42.0f;
            char time_text[64];
            std::snprintf(time_text, sizeof(time_text), "t = %.9f", getAccumTime());
            const float time_y = panel_min.y + ACCUM_TIME_MARGIN_TOP;
            // "t = " の部分を描画（左揃え）
            draw_list->AddText(ImVec2(panel_min.x + 20.0f, time_y), IM_COL32(0, 0, 0, 255), "t = ");
            // 数値部分を描画（AV テキストの x 座標と揃える）
            char time_value[64];
            std::snprintf(time_value, sizeof(time_value), "%.9f", getAccumTime());
            draw_list->AddText(ImVec2(panel_min.x + ACCUM_TIME_VALUE_X_OFFSET, time_y), IM_COL32(0, 0, 0, 255), time_value);
        }

        // Timeline アイテムの描画 (av_panel 内にクリップ)
        {
            constexpr float TIMELINE_ITEM_HEIGHT = 36.0f;
            constexpr float TIMELINE_ICON_SIZE = 32.0f;
            constexpr float TIMELINE_START_OFFSET = 68.0f;  // panel_min.y からのオフセット（累積時刻 t の分を追加）

            // av_panel のクリップ領域を設定
            draw_list->PushClipRect(
                ImVec2(panel_min.x, panel_min.y),
                panel_max,
                true
            );

            ImageManager& imageManager = ImageManager::GetInstance();

            for (size_t i = 0; i < getTimeline().size(); ++i)
            {
                const TimelineNode& node = getTimeline()[i];
                // ゲージ等の実体は UnitStatDetails 側に一元管理されているため、参照を取得する。
                const TeamState& team_state = getSimulationTeamState();
                const UnitStatDetails& node_unit = (node.team == 1)
                    ? team_state.ally[node.slotIndex]
                    : team_state.enemy[node.slotIndex];
                const float item_y = panel_min.y + TIMELINE_START_OFFSET + static_cast<float>(i) * TIMELINE_ITEM_HEIGHT;

                // アイテムの高さが panel_max.y を超える場合は描画しない
                if (item_y + TIMELINE_ITEM_HEIGHT > panel_max.y) break;

                const float icon_x = panel_min.x + 4.0f;
                const float icon_y = item_y + (TIMELINE_ITEM_HEIGHT - TIMELINE_ICON_SIZE) * 0.5f;

                // Actor かどうかで背景色を変更
                const bool isActor = (getActorTeam() == node.team && getActorSlot() == node.slotIndex);
                if (isActor)
                {
                    draw_list->AddRectFilled(
                        ImVec2(panel_min.x, item_y),
                        ImVec2(panel_max.x, item_y + TIMELINE_ITEM_HEIGHT),
                        IM_COL32(200, 230, 255, 255)
                    );
                }

                // ユニットの小さなアイコンを描画
                const GLuint unit_texture = imageManager.GetTexture(node_unit.character.id);
                if (unit_texture != 0)
                {
                    draw_list->AddImage(
                        (ImTextureID)(intptr_t)unit_texture,
                        ImVec2(icon_x, icon_y),
                        ImVec2(icon_x + TIMELINE_ICON_SIZE, icon_y + TIMELINE_ICON_SIZE)
                    );
                }

                // アイコンの円周をチーム色で強調
                {
                    const float icon_center_x = icon_x + TIMELINE_ICON_SIZE * 0.5f;
                    const float icon_center_y = icon_y + TIMELINE_ICON_SIZE * 0.5f;
                    const float icon_radius = TIMELINE_ICON_SIZE * 0.5f;
                    const ImU32 circle_color = (node.team == 1) ? IM_COL32(0, 100, 255, 255) : IM_COL32(255, 50, 50, 255);
                    draw_list->AddCircle(
                        ImVec2(icon_center_x, icon_center_y),
                        icon_radius,
                        circle_color,
                        0,
                        1.5f
                    );
                }

                const float text_x = icon_x + TIMELINE_ICON_SIZE + 6.0f;
				const float text_y = item_y + (TIMELINE_ITEM_HEIGHT - 14.0f) * 0.5f;

                // Actor は "Actor" テキストを表示、それ以外は AV の 小数第10位までの値を表示
                if (isActor)
                {
					const char* actor_text = "Actor";
                    ImGui::PushFont(g_font_bold);
					
					const ImVec2 actor_text_size = ImGui::CalcTextSize(actor_text);
					const float actor_text_y = item_y + (TIMELINE_ITEM_HEIGHT - actor_text_size.y) * 0.5f;
                    draw_list->AddText(ImVec2(text_x, actor_text_y), IM_COL32(0, 0, 180, 255), actor_text);
                    ImGui::PopFont();
                }
                else
                {
					// 一部の数値を、それが正確であることを示す整形を行う: 15.00 -> 15
                    char av_text[32];
                    std::snprintf(av_text, sizeof(av_text), "%.10f", static_cast<double>(node.av));
                    char* decimal_point = std::strchr(av_text, '.');
                    if (decimal_point != nullptr)
                    {
                        char* end = av_text + std::strlen(av_text) - 1;
                        while (end > decimal_point && *end == '0')
                        {
                            *end-- = '\0';
                        }
                        if (end == decimal_point)
                        {
                            *end = '\0';
                        }
                    }
                    draw_list->AddText(ImVec2(text_x, text_y), IM_COL32(0, 0, 0, 255), av_text);
                }
            }

            draw_list->PopClipRect();
        }
    }

    // -------------------------------------------------------
    // スロット描画用ヘルパー定数
    // -------------------------------------------------------

    constexpr float SIM_SLOT_SIZE = 88.0f;
    constexpr float SIM_SLOT_MARGIN = 24.0f;
    constexpr float SIM_SLOT_PITCH = SIM_SLOT_SIZE + SIM_SLOT_MARGIN;
    constexpr float SIM_CIRCLE_THICKNESS = 2.5f;
    constexpr int   SIM_CIRCLE_SEGMENTS  = 64;
    constexpr int   SIM_SLOT_COUNT = 5;
    constexpr float SIM_SLOT_BG_ROUNDING = 8.0f;

    // スロット背景色 (formationView のパネル色を参照)
    // Ally: 暗い青系 (51,60,73,255)
    // Enemy: 暗い赤系 (122,62,70,255)
    constexpr ImU32 SIM_SLOT_BG_ALLY  = IM_COL32(51, 60, 73, 255);
    constexpr ImU32 SIM_SLOT_BG_ENEMY = IM_COL32(122, 62, 70, 255);

    // av_panel の右端 X（スクリーン座標）
    const ImVec2 child_screen_pos = ImGui::GetCursorScreenPos();
    constexpr float AV_PANEL_WIDTH   = 160.0f;
    constexpr float AV_PANEL_MARGIN  = 12.0f;
    constexpr float SLOT_MARGIN_RIGHT = 16.0f;
    const float slots_start_x =
        child_screen_pos.x + AV_PANEL_MARGIN + AV_PANEL_WIDTH + SLOT_MARGIN_RIGHT;

    // HPバーはスロット直下にカプセル形状で描画し、現在HPを重ねて表示する。
    constexpr float HP_BAR_HEIGHT = 8.0f;
    constexpr float HP_BAR_GAP = -5.0f;
    constexpr float HP_BAR_ROUNDING = HP_BAR_HEIGHT * 0.5f;
    constexpr ImU32 HP_BAR_BACKGROUND = IM_COL32(35, 35, 35, 255);
    constexpr ImU32 HP_BAR_COLOR = IM_COL32(154, 205, 50, 255); // HP>=40%
	constexpr ImU32 HP_BAR_WARNING_COLOR = IM_COL32(251, 208, 19, 255); // HP>=10%
	constexpr ImU32 HP_BAR_DANGER_COLOR = IM_COL32(223, 14, 9, 255);
    const auto draw_hp_bar = [&](ImDrawList* draw_list, float x, float y, const UnitStatDetails& slot_data)
    {
        if (slot_data.character.id == 0)
        {
            return;
        }

        const float ratio = (slot_data.maxHP > 0)
            ? std::clamp(static_cast<float>(slot_data.hp) /
                         static_cast<float>(slot_data.maxHP), 0.0f, 1.0f)
            : 0.0f;
        const float width = SIM_SLOT_SIZE * 0.9f;
		const float bar_x = x + (SIM_SLOT_SIZE - width) * 0.5f;
		
		const ImU32 bar_color = (ratio >= 0.4f)
			? HP_BAR_COLOR
			: (ratio >= 0.1f)
				? HP_BAR_WARNING_COLOR
				: HP_BAR_DANGER_COLOR;
				
        draw_list->AddRectFilled(
            ImVec2(bar_x, y),
            ImVec2(bar_x + width, y + HP_BAR_HEIGHT),
            HP_BAR_BACKGROUND,
            HP_BAR_ROUNDING
        );
        if (ratio > 0.0f)
        {
            draw_list->AddRectFilled(
                ImVec2(bar_x, y),
                ImVec2(bar_x + width * ratio, y + HP_BAR_HEIGHT),
                bar_color,
                HP_BAR_ROUNDING
            );
        }

        // 現在HPをバーの手前側に重ねて表示する。右端を固定し、桁数が
        // 変わっても右揃えになるよう、文字列幅分だけ左へずらす。
        char hp_text[32];
        std::snprintf(hp_text, sizeof(hp_text), "%d", slot_data.hp);
        constexpr float HP_TEXT_SIZE = 22.0f;
        const ImVec2 hp_text_size = ImGui::GetFont()->CalcTextSizeA(
            HP_TEXT_SIZE, FLT_MAX, 0.0f, hp_text
        );
        const ImVec2 hp_text_pos(
            bar_x + width - 3.0f - hp_text_size.x,
            y + (HP_BAR_HEIGHT - hp_text_size.y) * 0.5f
        );

        // 背景色がHP残量によって変化するため、黒い縁取りで視認性を確保する。
        const ImU32 hp_text_outline = IM_COL32(0, 0, 0, 255);
        draw_list->AddText(nullptr, HP_TEXT_SIZE,
            ImVec2(hp_text_pos.x - 1.0f, hp_text_pos.y), hp_text_outline, hp_text);
        draw_list->AddText(nullptr, HP_TEXT_SIZE,
            ImVec2(hp_text_pos.x + 1.0f, hp_text_pos.y), hp_text_outline, hp_text);
        draw_list->AddText(nullptr, HP_TEXT_SIZE,
            ImVec2(hp_text_pos.x, hp_text_pos.y - 1.0f), hp_text_outline, hp_text);
        draw_list->AddText(nullptr, HP_TEXT_SIZE,
            ImVec2(hp_text_pos.x, hp_text_pos.y + 1.0f), hp_text_outline, hp_text);
        draw_list->AddText(nullptr, HP_TEXT_SIZE, hp_text_pos,
            IM_COL32(255, 255, 255, 255), hp_text);
    };

    // -------------------------------------------------------
    // 敵スロット: av_panel 右隣、domain1 の top から 100px マージンで横一列
    // -------------------------------------------------------

    {
        constexpr float SLOT_MARGIN_TOP = 100.0f;
        const float slot_y = SLOT_MARGIN_TOP;

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImageManager& imageManager = ImageManager::GetInstance();

        // 敵スロット全体を覆う背景矩形（5スロット分、上下左右 8px ゆとり）
        {
            constexpr float SIM_SLOT_BG_PAD = 8.0f;
            const float bg_width = SIM_SLOT_COUNT * SIM_SLOT_PITCH - SIM_SLOT_MARGIN;
            draw_list->AddRectFilled(
                ImVec2(slots_start_x - SIM_SLOT_BG_PAD, slot_y - SIM_SLOT_BG_PAD),
                ImVec2(slots_start_x + bg_width + SIM_SLOT_BG_PAD, slot_y + SIM_SLOT_SIZE + SIM_SLOT_BG_PAD),
                SIM_SLOT_BG_ENEMY,
                SIM_SLOT_BG_ROUNDING
            );
        }

        // Actor の緑強調リング色 (formationView の選択中スロット色と同一)
        constexpr ImU32 ACTOR_HIGHLIGHT_COLOR = IM_COL32(80, 220, 100, 255);

        for (int i = 0; i < SIM_SLOT_COUNT; ++i)
        {
            // 表示位置 i に内部スロット (4 - i) を配置する。内部のslotIndexは変更しない。
            const int slot_index = SIM_SLOT_COUNT - 1 - i;
            const float slot_x = slots_start_x + i * SIM_SLOT_PITCH;
            const ImVec2 screen_slot_center(
                slot_x + SIM_SLOT_SIZE * 0.5f,
                slot_y + SIM_SLOT_SIZE * 0.5f
            );

            const UnitStatDetails& slotData = getSimulationTeamState().enemy[slot_index];
            const bool has_unit = (slotData.character.id != 0);

            if (has_unit)
            {
                const Character& unit = slotData.character;
                const GLuint unit_texture = imageManager.GetTexture(unit.id);

                if (unit_texture != 0)
                {
                    const ImVec2 img_min(slot_x, slot_y);
                    const ImVec2 img_max(slot_x + SIM_SLOT_SIZE, slot_y + SIM_SLOT_SIZE);
                    const ImU32 portrait_tint = (slotData.hp <= 0)
                        ? IM_COL32(165, 165, 165, 255)
                        : IM_COL32(255, 255, 255, 255);
                    draw_list->AddImage(
                        (ImTextureID)(intptr_t)unit_texture,
                        img_min,
                        img_max,
                        ImVec2(0.0f, 0.0f),
                        ImVec2(1.0f, 1.0f),
                        portrait_tint
                    );
                }

                // Actor の場合、緑の強調リングを描画
                if (getActorTeam() == 0 && getActorSlot() == slot_index)
                {
                    const float ring_radius = SIM_SLOT_SIZE * 0.5f;
                    draw_list->AddCircle(
                        screen_slot_center,
                        ring_radius,
                        ACTOR_HIGHLIGHT_COLOR,
                        0,
                        3.0f
                    );
                }
            }
            else
            {
                const float circle_radius =
                    SIM_SLOT_SIZE * 0.5f - SIM_CIRCLE_THICKNESS * 0.5f;
                draw_list->AddCircle(
                    screen_slot_center,
                    circle_radius,
                    IM_COL32(200, 200, 200, 255),
                    SIM_CIRCLE_SEGMENTS,
                    SIM_CIRCLE_THICKNESS
                );
            }

        }
    }

    // -------------------------------------------------------
    // 味方スロット: av_panel 右から 16px マージン、domain1 の bottom から 24px マージンで横一列
    // -------------------------------------------------------

    {
        constexpr float SLOT_MARGIN_BOTTOM = 24.0f;
        const float slot_y = DEFAULT_SPLIT_Y - SLOT_MARGIN_BOTTOM - SIM_SLOT_SIZE;

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImageManager& imageManager = ImageManager::GetInstance();

        // 味方スロット全体を覆う背景矩形（5スロット分、上下左右 8px ゆとり）
        {
            constexpr float SIM_SLOT_BG_PAD = 8.0f;
            const float bg_width = SIM_SLOT_COUNT * SIM_SLOT_PITCH - SIM_SLOT_MARGIN;
            draw_list->AddRectFilled(
                ImVec2(slots_start_x - SIM_SLOT_BG_PAD, slot_y - SIM_SLOT_BG_PAD),
                ImVec2(slots_start_x + bg_width + SIM_SLOT_BG_PAD, slot_y + SIM_SLOT_SIZE + SIM_SLOT_BG_PAD),
                SIM_SLOT_BG_ALLY,
                SIM_SLOT_BG_ROUNDING
            );
        }

        // Actor の緑強調リング色 (formationView の選択中スロット色と同一)
        constexpr ImU32 ACTOR_HIGHLIGHT_COLOR = IM_COL32(80, 220, 100, 255);

        for (int i = 0; i < SIM_SLOT_COUNT; ++i)
        {
            const float slot_x = slots_start_x + i * SIM_SLOT_PITCH;
            const ImVec2 screen_slot_center(
                slot_x + SIM_SLOT_SIZE * 0.5f,
                slot_y + SIM_SLOT_SIZE * 0.5f
            );

            const UnitStatDetails& slotData = getSimulationTeamState().ally[i];
            const bool has_unit = (slotData.character.id != 0);

            if (has_unit)
            {
                const Character& unit = slotData.character;
                const GLuint unit_texture = imageManager.GetTexture(unit.id);

                if (unit_texture != 0)
                {
                    const ImVec2 img_min(slot_x, slot_y);
                    const ImVec2 img_max(slot_x + SIM_SLOT_SIZE, slot_y + SIM_SLOT_SIZE);
                    const ImU32 portrait_tint = (slotData.hp <= 0)
                        ? IM_COL32(165, 165, 165, 255)
                        : IM_COL32(255, 255, 255, 255);
                    draw_list->AddImage(
                        (ImTextureID)(intptr_t)unit_texture,
                        img_min,
                        img_max,
                        ImVec2(0.0f, 0.0f),
                        ImVec2(1.0f, 1.0f),
                        portrait_tint
                    );
                }

                // Actor の場合、緑の強調リングを描画
                if (getActorTeam() == 1 && getActorSlot() == i)
                {
                    const float ring_radius = SIM_SLOT_SIZE * 0.5f;
                    draw_list->AddCircle(
                        screen_slot_center,
                        ring_radius,
                        ACTOR_HIGHLIGHT_COLOR,
                        0,
                        3.0f
                    );
                }
            }
            else
            {
                const float circle_radius =
                    SIM_SLOT_SIZE * 0.5f - SIM_CIRCLE_THICKNESS * 0.5f;
                draw_list->AddCircle(
                    screen_slot_center,
                    circle_radius,
                    IM_COL32(200, 200, 200, 255),
                    SIM_CIRCLE_SEGMENTS,
                    SIM_CIRCLE_THICKNESS
                );
            }

        }
    }

    // -------------------------------------------------------
    // SkillType セグメント選択（Action Log と同様の手動描画・クリック判定）
    // -------------------------------------------------------

    {
        constexpr float SKILL_PANEL_SIZE = SIM_SLOT_SIZE;
        constexpr float SKILL_PANEL_ROUNDING = 8.0f;
        constexpr float SKILL_HEADER_HEIGHT = 26.0f;
        constexpr float SKILL_ROW_HEIGHT =
            (SKILL_PANEL_SIZE - SKILL_HEADER_HEIGHT) / 3.0f;
        constexpr float SIM_SLOT_BG_PAD = 8.0f;
        constexpr float SIM_SLOT_BG_WIDTH =
            SIM_SLOT_COUNT * SIM_SLOT_PITCH - SIM_SLOT_MARGIN;
        constexpr float SLOT_MARGIN_BOTTOM = 24.0f;
        const float slot_y =
            DEFAULT_SPLIT_Y - SLOT_MARGIN_BOTTOM - SIM_SLOT_SIZE;

        const float ally_bg_right =
            slots_start_x + SIM_SLOT_BG_WIDTH + SIM_SLOT_BG_PAD;
        const ImVec2 window_pos = ImGui::GetWindowPos();
        const ImVec2 window_size = ImGui::GetWindowSize();
        const float domain1_right = window_pos.x + window_size.x;
        const float panel_center_x = (ally_bg_right + domain1_right) * 0.5f;
        const float panel_x = panel_center_x - SKILL_PANEL_SIZE * 0.5f;
        const ImVec2 panel_min(panel_x, slot_y);
        const ImVec2 panel_max(
            panel_x + SKILL_PANEL_SIZE,
            slot_y + SKILL_PANEL_SIZE
        );

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        const ImU32 panel_background = IM_COL32(245, 245, 245, 255);
        const ImU32 text_color = IM_COL32(35, 35, 35, 255);
        const ImU32 separator_color = IM_COL32(175, 175, 175, 255);
        const ImU32 ba_selected_color = IM_COL32(200, 230, 255, 255);
        const ImU32 bs_selected_color = IM_COL32(238, 151, 58, 255);
        [[maybe_unused]] constexpr ImU32 sa_background_color = IM_COL32(220, 70, 70, 255);
        const ImU32 disabled_text_color = IM_COL32(145, 145, 145, 255);

        // パネル全体を先に描画し、各帯をその内側に配置する。
        draw_list->AddRectFilled(
            panel_min, panel_max, panel_background, SKILL_PANEL_ROUNDING
        );

        const char* title_text = "[SkillType]";
        const ImVec2 title_size = ImGui::CalcTextSize(title_text);
        draw_list->AddText(
            ImVec2(panel_center_x - title_size.x * 0.5f,
                   slot_y + (SKILL_HEADER_HEIGHT - title_size.y) * 0.5f),
            text_color,
            title_text
        );
        draw_list->AddLine(
            ImVec2(panel_min.x + 4.0f, slot_y + SKILL_HEADER_HEIGHT),
            ImVec2(panel_max.x - 4.0f, slot_y + SKILL_HEADER_HEIGHT),
            separator_color,
            1.0f
        );

        const SkillType selected = getCurrentSkillType();
        const SkillType row_types[] = { SkillType::BA, SkillType::BS, SkillType::SA };
        const char* row_labels[] = { "BA", "BS", "SA" };
        const ImGuiIO& io = ImGui::GetIO();
        const bool within_domain1 =
            io.MousePos.x >= window_pos.x &&
            io.MousePos.x < window_pos.x + window_size.x &&
            io.MousePos.y >= window_pos.y &&
            io.MousePos.y < window_pos.y + window_size.y;

        for (int i = 0; i < 3; ++i)
        {
            const float row_top = slot_y + SKILL_HEADER_HEIGHT + i * SKILL_ROW_HEIGHT;
            const float row_bottom = row_top + SKILL_ROW_HEIGHT;
            const bool is_selected = (selected == row_types[i]);
            const bool is_disabled = (row_types[i] == SkillType::SA);

            // Action Log と同じく、未選択行には別のグレー背景を描かない。
            // これにより各行と外側パネルの背景色が一致し、色の段差が発生しない。
            // 選択帯は四辺をパネル内側へ inset し、外側の角丸背景からはみ出さない形にする。
            if (is_selected && !is_disabled)
            {
                constexpr float HIGHLIGHT_INSET_X = 4.0f;
                constexpr float HIGHLIGHT_INSET_Y = 1.0f;
                constexpr float HIGHLIGHT_ROUNDING = 4.0f;
                draw_list->AddRectFilled(
                    ImVec2(panel_min.x + HIGHLIGHT_INSET_X, row_top + HIGHLIGHT_INSET_Y),
                    ImVec2(panel_max.x - HIGHLIGHT_INSET_X, row_bottom - HIGHLIGHT_INSET_Y),
                    (row_types[i] == SkillType::BS)
                        ? bs_selected_color
                        : ba_selected_color,
                    HIGHLIGHT_ROUNDING
                );
            }
            if (i > 0)
            {
                draw_list->AddLine(
                    ImVec2(panel_min.x + 6.0f, row_top),
                    ImVec2(panel_max.x - 6.0f, row_top),
                    separator_color,
                    1.0f
                );
            }

            const ImVec2 label_size = ImGui::CalcTextSize(row_labels[i]);
            const ImU32 label_color = is_disabled
                ? disabled_text_color
                : (is_selected ? text_color : IM_COL32(85, 85, 85, 255));
            draw_list->AddText(
                ImVec2(panel_center_x - label_size.x * 0.5f,
                       row_top + (SKILL_ROW_HEIGHT - label_size.y) * 0.5f),
                label_color,
                row_labels[i]
            );

            // SA は表示のみ。クリックしても選択状態を変更しない。
            const bool hovered =
                within_domain1 &&
                io.MousePos.x >= panel_min.x && io.MousePos.x <= panel_max.x &&
                io.MousePos.y >= row_top && io.MousePos.y < row_bottom;
            if (!is_disabled && hovered)
            {
                ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
                if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) &&
                    row_types[i] != selected)
                {
                    toggleCurrentSkillType();
                }
            }
        }
    }

    // -------------------------------------------------------
    // ターゲット選択: 敵スロット・味方スロットのクリック判定と hover 強調
    // Actor の相手チームのスロットをクリックするとターゲットが選択され、ターンが進行
    // -------------------------------------------------------

    {
        const ImGuiIO& io = ImGui::GetIO();
        const ImVec2 window_pos = ImGui::GetWindowPos();
        const float mouse_x = io.MousePos.x - window_pos.x;
        const float mouse_y = io.MousePos.y - window_pos.y;

        // domain1 の表示範囲外ではクリック判定を行わない
        const bool within_domain1 =
            mouse_x >= 0.0f && mouse_x < split_x &&
            mouse_y >= 0.0f && mouse_y < split_y;

        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        // ターゲット候補スロットの hover 強調色 (水色)
        constexpr ImU32 TARGET_HOVER_COLOR = IM_COL32(100, 245, 255, 255);

        // 敵スロットのクリック判定と hover 強調
        {
            constexpr float SLOT_MARGIN_TOP = 100.0f;
            const float slot_y = SLOT_MARGIN_TOP;
            for (int i = 0; i < SIM_SLOT_COUNT; ++i)
            {
                // 画面上の左から i 番目に対応する内部スロット。内部順序は変更しない。
                const int slot_index = SIM_SLOT_COUNT - 1 - i;
                const float slot_x = slots_start_x + i * SIM_SLOT_PITCH;
                if (getSimulationTeamState().enemy[slot_index].character.id != 0 &&
                    getSimulationTeamState().enemy[slot_index].hp > 0)
                {
                    const bool hovered =
                        mouse_x >= slot_x && mouse_x <= slot_x + SIM_SLOT_SIZE &&
                        mouse_y >= slot_y && mouse_y <= slot_y + SIM_SLOT_SIZE;

                    // Actor が味方 (team=1) の場合、敵スロットはターゲット候補
                    if (hovered && within_domain1 && getActorTeam() == 1 && getActorSlot() >= 0)
                    {
                        const ImVec2 slot_center(slot_x + SIM_SLOT_SIZE * 0.5f, slot_y + SIM_SLOT_SIZE * 0.5f);
                        const float ring_radius = SIM_SLOT_SIZE * 0.5f;
                        draw_list->AddCircle(
                            slot_center,
                            ring_radius,
                            TARGET_HOVER_COLOR,
                            0,
                            3.0f
                        );

                        // BS（Proximity）の場合、対象の左右に隣接する占有スロットを薄い色で強調。
                        if (getCurrentSkillType() == SkillType::BS)
                        {
                            constexpr ImU32 PROXIMITY_HOVER_COLOR = IM_COL32(125, 225, 235, 248);
                            for (const int adjacent_slot : { slot_index - 1, slot_index + 1 })
                            {
                                if (adjacent_slot < 0 || adjacent_slot >= SIM_SLOT_COUNT ||
                                    getSimulationTeamState().enemy[adjacent_slot].character.id == 0 ||
                                    getSimulationTeamState().enemy[adjacent_slot].hp <= 0)
                                {
                                    continue;
                                }

                                const float adjacent_x = slots_start_x + (SIM_SLOT_COUNT - 1 - adjacent_slot) * SIM_SLOT_PITCH;
                                const ImVec2 adjacent_center(
                                    adjacent_x + SIM_SLOT_SIZE * 0.5f,
                                    slot_y + SIM_SLOT_SIZE * 0.5f
                                );
                                draw_list->AddCircle(
                                    adjacent_center,
                                    ring_radius,
                                    PROXIMITY_HOVER_COLOR,
                                    0,
                                    2.0f
                                );
                            }
                        }

                        const bool clicked = hovered && within_domain1 && ImGui::IsMouseClicked(ImGuiMouseButton_Left);
                        if (clicked)
                        {
                            proceedTurnWithHistory(getCurrentSkillType(), 0, slot_index);
                        }
                    }
                }
            }
        }

        // 味方スロットのクリック判定と hover 強調
        {
            constexpr float SLOT_MARGIN_BOTTOM = 24.0f;
            const float slot_y = DEFAULT_SPLIT_Y - SLOT_MARGIN_BOTTOM - SIM_SLOT_SIZE;
            for (int i = 0; i < SIM_SLOT_COUNT; ++i)
            {
                const float slot_x = slots_start_x + i * SIM_SLOT_PITCH;
                if (getSimulationTeamState().ally[i].character.id != 0 &&
                    getSimulationTeamState().ally[i].hp > 0)
                {
                    const bool hovered =
                        mouse_x >= slot_x && mouse_x <= slot_x + SIM_SLOT_SIZE &&
                        mouse_y >= slot_y && mouse_y <= slot_y + SIM_SLOT_SIZE;

                    // Actor が敵 (team=0) の場合、味方スロットはターゲット候補
                    if (hovered && within_domain1 && getActorTeam() == 0 && getActorSlot() >= 0)
                    {
                        const ImVec2 slot_center(slot_x + SIM_SLOT_SIZE * 0.5f, slot_y + SIM_SLOT_SIZE * 0.5f);
                        const float ring_radius = SIM_SLOT_SIZE * 0.5f;
                        draw_list->AddCircle(
                            slot_center,
                            ring_radius,
                            TARGET_HOVER_COLOR,
                            0,
                            3.0f
                        );

                        // BS（Proximity）の場合、対象の左右に隣接する占有スロットを薄い色で強調。
                        if (getCurrentSkillType() == SkillType::BS)
                        {
                            constexpr ImU32 PROXIMITY_HOVER_COLOR = IM_COL32(125, 225, 235, 248);
                            for (const int adjacent_slot : { i - 1, i + 1 })
                            {
                                if (adjacent_slot < 0 || adjacent_slot >= SIM_SLOT_COUNT ||
                                    getSimulationTeamState().ally[adjacent_slot].character.id == 0 ||
                                    getSimulationTeamState().ally[adjacent_slot].hp <= 0)
                                {
                                    continue;
                                }

                                const float adjacent_x = slots_start_x + adjacent_slot * SIM_SLOT_PITCH;
                                const ImVec2 adjacent_center(
                                    adjacent_x + SIM_SLOT_SIZE * 0.5f,
                                    slot_y + SIM_SLOT_SIZE * 0.5f
                                );
                                draw_list->AddCircle(
                                    adjacent_center,
                                    ring_radius,
                                    PROXIMITY_HOVER_COLOR,
                                    0,
                                    2.0f
                                );
                            }
                        }

                        const bool clicked = hovered && within_domain1 && ImGui::IsMouseClicked(ImGuiMouseButton_Left);
                        if (clicked)
                        {
                            proceedTurnWithHistory(getCurrentSkillType(), 1, i);
                        }
                    }
                }
            }
        }
    }

    // HPバーは対象・隣接対象の強調リングより後に描画し、リングがバーの上に
    // 重ならないようにする。アイコンと強調リングの前後関係は維持する。
    {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        constexpr float ENEMY_SLOT_Y = 100.0f;
        constexpr float ALLY_SLOT_Y = DEFAULT_SPLIT_Y - 24.0f - SIM_SLOT_SIZE;
        for (int i = 0; i < SIM_SLOT_COUNT; ++i)
        {
            const int enemy_slot_index = SIM_SLOT_COUNT - 1 - i;
            const float slot_x = slots_start_x + i * SIM_SLOT_PITCH;
            draw_hp_bar(
                draw_list,
                slot_x,
                ENEMY_SLOT_Y + SIM_SLOT_SIZE + HP_BAR_GAP,
                getSimulationTeamState().enemy[enemy_slot_index]
            );
            draw_hp_bar(
                draw_list,
                slot_x,
                ALLY_SLOT_Y + SIM_SLOT_SIZE + HP_BAR_GAP,
                getSimulationTeamState().ally[i]
            );
        }
    }

    ImGui::EndChild();


    // =======================================================
    // domain2
    // =======================================================

    ImGui::SameLine(0.0f, 0.0f);

    ImGui::BeginChild(
        "domain2",
        ImVec2(
            static_cast<float>(WINDOW_WIDTH) - split_x,
            split_y
        ),
        true,
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse |
        ImGuiWindowFlags_NoBackground
    );

    // -------------------------------------------------------
    // actionLog_panel: domain2 左側の角丸白矩形（av_panel と同様の固定座標）
    // x 座標のみ split_x から相対計算、y 座標・width・height は av_panel と同一
    // -------------------------------------------------------

    {
        constexpr float ACTION_LOG_PANEL_WIDTH   = 240.0f;
        constexpr float ACTION_LOG_PANEL_MARGIN  = 12.0f;
        constexpr float ACTION_LOG_PANEL_ROUNDING = 8.0f;

        // domain2 のスクリーン座標を取得
        const ImVec2 domain2_screen_pos = ImGui::GetCursorScreenPos();

        // x 座標は split_x からマージン、y 座標は av_panel と同じ固定値
        const ImVec2 panel_min(
            domain2_screen_pos.x + ACTION_LOG_PANEL_MARGIN,
            domain2_screen_pos.y + ACTION_LOG_PANEL_MARGIN
        );
        const ImVec2 panel_max(
            domain2_screen_pos.x + ACTION_LOG_PANEL_MARGIN + ACTION_LOG_PANEL_WIDTH,
            domain2_screen_pos.y + DEFAULT_SPLIT_Y - ACTION_LOG_PANEL_MARGIN
        );

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        draw_list->AddRectFilled(
            panel_min,
            panel_max,
            IM_COL32(245, 245, 245, 255),
            ACTION_LOG_PANEL_ROUNDING
        );

        // "Action Log" テキスト: panel 上側中央寄せ、太字、黒色
        {
            constexpr float ACTION_LOG_TEXT_MARGIN_TOP = 16.0f;

            const char* action_log_text = "Action Log";

            ImGui::PushFont(g_font_bold);
            ImVec2 text_size = ImGui::CalcTextSize(action_log_text);
            ImGui::PopFont();

            const float text_x = panel_min.x + (ACTION_LOG_PANEL_WIDTH - text_size.x) * 0.5f;
            const float text_y = panel_min.y + ACTION_LOG_TEXT_MARGIN_TOP;

            ImGui::PushFont(g_font_bold);
            ImGui::SetCursorScreenPos(ImVec2(text_x, text_y));
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 0, 0, 255));
            ImGui::TextUnformatted(action_log_text);
            ImGui::PopStyleColor();
            ImGui::PopFont();
        }

        // -------------------------------------------------------
		// Action Log アイテムの描画
		// ネイティブスクロールバー・クリックによる履歴移動
		// -------------------------------------------------------

		{
			constexpr float ACTION_LOG_ITEM_HEIGHT = 20.0f;
			constexpr float ACTION_LOG_START_OFFSET = 52.0f;
			constexpr float ACTION_LOG_TEXT_MARGIN_X = 14.0f;

			const int log_count =
				static_cast<int>(getActionLog().size());

			// -------------------------------------------------------
			// actionLog_panel 内のログ表示領域
			// -------------------------------------------------------

			const float content_top_y =
				panel_min.y + ACTION_LOG_START_OFFSET;

			const ImVec2 domain2_window_pos =
				ImGui::GetWindowPos();

			const ImVec2 domain2_window_size =
				ImGui::GetWindowSize();

			// domain2 自体の下端を越えないようにする
			const float domain2_bottom_y =
				domain2_window_pos.y +
				domain2_window_size.y -
				ImGui::GetStyle().WindowBorderSize;

			const float visible_bottom_y =
				std::min(panel_max.y, domain2_bottom_y);

			const float scroll_region_height =
				std::max(
					0.0f,
					visible_bottom_y - content_top_y
				);

			const float panel_width =
				panel_max.x - panel_min.x;


			// -------------------------------------------------------
			// ログ表示領域
			// -------------------------------------------------------

			if (scroll_region_height > 1.0f)
			{
				const ImVec2 content_min(
					panel_min.x,
					content_top_y
				);

				// Child を actionLog_panel 内に配置する
				ImGui::SetCursorScreenPos(content_min);

				ImGui::PushStyleVar(
					ImGuiStyleVar_WindowPadding,
					ImVec2(0.0f, 0.0f)
				);

				ImGui::PushStyleVar(
					ImGuiStyleVar_ItemSpacing,
					ImVec2(0.0f, 0.0f)
				);

 			ImGui::PushStyleVar(
 				ImGuiStyleVar_FramePadding,
 				ImVec2(ACTION_LOG_TEXT_MARGIN_X, 1.0f)
 			);

 			// スクロールバーの色を背景矩形に合わせる
 			ImGui::PushStyleColor(
 				ImGuiCol_ScrollbarBg,
 				IM_COL32(245, 245, 245, 255)
 			);

 			ImGui::PushStyleColor(
 				ImGuiCol_ScrollbarGrab,
 				IM_COL32(220, 220, 220, 255)
 			);

 			ImGui::PushStyleColor(
 				ImGuiCol_ScrollbarGrabHovered,
 				IM_COL32(200, 200, 200, 255)
 			);

 			ImGui::PushStyleColor(
 				ImGuiCol_ScrollbarGrabActive,
 				IM_COL32(180, 180, 180, 255)
 			);

			// 最後に描画した時点の履歴インデックス
			const int previous_history_index =
				g_lastAutoScrollHistoryIndex;

			// ---------------------------------------------------
			// キー操作をログ描画より前に処理
			// ---------------------------------------------------

			if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow))
			{
				backupTurn();
			}

			if (ImGui::IsKeyPressed(ImGuiKey_RightArrow))
			{
				forwardTurn();
			}

			// ---------------------------------------------------
			// マウスクリックによる履歴参照
			// スクロールバー領域はクリック判定から除外
			// ---------------------------------------------------

			// 末尾に1行分の余白を設け、最新ログを下から2行目に配置できるようにする
			const float log_content_height =
				static_cast<float>(log_count + 1) * ACTION_LOG_ITEM_HEIGHT;

			const bool scrollbar_visible =
				log_content_height > scroll_region_height + 0.5f;

			const float scrollbar_width =
				scrollbar_visible
					? ImGui::GetStyle().ScrollbarSize
					: 0.0f;

			const float log_clickable_right =
				content_min.x + panel_width - scrollbar_width;

			const ImVec2 mouse_pos =
				ImGui::GetIO().MousePos;

			if (
				ImGui::IsMouseClicked(ImGuiMouseButton_Left) &&
				mouse_pos.x >= content_min.x &&
				mouse_pos.x < log_clickable_right &&
				mouse_pos.y >= content_top_y &&
				mouse_pos.y < content_top_y + scroll_region_height
			)
			{
				const float row_content_y =
					mouse_pos.y - content_top_y + g_actionLogScrollY;

				const int clicked_log_index =
					static_cast<int>(
						std::floor(row_content_y / ACTION_LOG_ITEM_HEIGHT)
					);

				const int clicked_history_index =
					clicked_log_index + 1;

				if (
					clicked_log_index >= 0 &&
					clicked_log_index < log_count &&
					clicked_history_index <
						static_cast<int>(getHistoryCount())
				)
				{
					restoreHistoryStep(clicked_history_index);
				}
			}

			// ---------------------------------------------------
			// 今回の履歴移動に対応するスクロール位置を計算
			// ---------------------------------------------------

			const float max_scroll_y =
				std::max(
					0.0f,
					log_content_height - scroll_region_height
				);

			const float current_scroll_y =
				std::clamp(
					g_actionLogScrollY,
					0.0f,
					max_scroll_y
				);

			float next_scroll_y = current_scroll_y;

			const int current_log_index =
				getHistoryIndex() - 1;

			if (
				previous_history_index >= 0 &&
				getHistoryIndex() != previous_history_index
			)
			{
				const int direction =
					getHistoryIndex() < previous_history_index ? -1 : 1;

				if (current_log_index < 0)
				{
					// 初期状態へ戻った場合は先頭へ戻す
					next_scroll_y = 0.0f;
				}
				else
				{
					const int first_visible_index =
						std::max(
							0,
							static_cast<int>(
								std::floor(
									current_scroll_y / ACTION_LOG_ITEM_HEIGHT
								)
							)
						);

					const int last_visible_index =
						std::max(
							first_visible_index,
							static_cast<int>(
								std::floor(
									(
										current_scroll_y +
										scroll_region_height -
										0.001f
									) / ACTION_LOG_ITEM_HEIGHT
								)
							)
						);

					if (
						direction < 0 &&
						current_log_index <= first_visible_index
					)
					{
						// 過去へ：選択行が先頭行に来たら1行分上へ移動し、2行目に戻す
						next_scroll_y =
							std::max(
								0.0f,
								current_scroll_y - ACTION_LOG_ITEM_HEIGHT
							);
					}
					else if (
						direction > 0 &&
						current_log_index >= last_visible_index
					)
					{
						// 未来へ：選択行が最終表示行に来たら1行分下へ移動し、下から2行目に戻す
						next_scroll_y =
							std::min(
								max_scroll_y,
								current_scroll_y + ACTION_LOG_ITEM_HEIGHT
							);
					}
				}
			}

			// ---------------------------------------------------
			// 子ウィンドウを開始する前に、スクロール位置を予約
			// ---------------------------------------------------

			const bool request_scroll =
				std::fabs(next_scroll_y - current_scroll_y) > 0.01f;

			g_lastAutoScrollHistoryIndex = getHistoryIndex();

			ImGui::SetCursorScreenPos(content_min);
			
			if (request_scroll)
			{
				ImGui::SetNextWindowScroll(
					ImVec2(-1.0f, next_scroll_y)
				);
			}

 			ImGui::BeginChild(
 				"ActionLogScrollRegion",
 				ImVec2(panel_width, scroll_region_height),
 				false,
 				ImGuiWindowFlags_NoBackground |
 				ImGuiWindowFlags_NoSavedSettings
 			);

 				// ---------------------------------------------------
 				// テキスト色
 				// ---------------------------------------------------

 				ImGui::PushStyleColor(
 					ImGuiCol_Text,
 					IM_COL32(0, 0, 0, 255)
 				);

 				// ---------------------------------------------------
 				// ログ行（手動描画：角丸クリッピング対応）
 				// ---------------------------------------------------

 				const ImVec2 child_screen_pos =
 					ImGui::GetCursorScreenPos();

 				const float log_row_x =
 					child_screen_pos.x + ACTION_LOG_TEXT_MARGIN_X;
 				// 強調色の角丸半径（背景矩形の角丸より小さくする）
 				constexpr float HIGHLIGHT_ROUNDING = 4.0f;

 				for (int i = 0; i < log_count; ++i)
 				{
 					const bool selected =
 						(i == current_log_index);

 					const float row_y =
 						child_screen_pos.y +
 						i * ACTION_LOG_ITEM_HEIGHT;

 					const float row_bottom =
 						row_y + ACTION_LOG_ITEM_HEIGHT;

 					// 強調色の矩形を描画（角丸付き）
 					if (selected)
 					{
 						ImDrawList* log_draw_list =
 							ImGui::GetWindowDrawList();

 						log_draw_list->AddRectFilled(
 							ImVec2(
 								child_screen_pos.x,
 								row_y
 							),
 							ImVec2(
 								child_screen_pos.x + panel_width,
 								row_bottom
 							),
 							IM_COL32(200, 230, 255, 255),
 							HIGHLIGHT_ROUNDING
 						);
 					}

 					// テキストを描画
 					{
 						const float text_y =
 							row_y +
 							(ACTION_LOG_ITEM_HEIGHT - 14.0f) * 0.5f;

 						ImDrawList* log_draw_list =
 							ImGui::GetWindowDrawList();

 						log_draw_list->AddText(
 							ImVec2(log_row_x, text_y),
 							IM_COL32(0, 0, 0, 255),
 							getActionLog()[i].c_str()
 						);
 					}

 					// カーソル形状
 					{
 						const ImVec2 mouse_pos =
 							ImGui::GetIO().MousePos;

 						const bool hovered =
 							mouse_pos.x >= child_screen_pos.x &&
 							mouse_pos.x < log_clickable_right &&
 							mouse_pos.y >= row_y &&
 							mouse_pos.y < row_bottom;

 						if (hovered)
 						{
 							ImGui::SetMouseCursor(
 								ImGuiMouseCursor_Arrow
 							);
 						}
 					}

 					// ログ行の領域を ImGui に登録し、カーソルを次の行へ進める
					ImGui::Dummy(
						ImVec2(panel_width, ACTION_LOG_ITEM_HEIGHT)
					);
 				}

				// 計算上の余白と同じ1行を実際の ImGui コンテンツにも追加
				ImGui::Dummy(
					ImVec2(panel_width, ACTION_LOG_ITEM_HEIGHT)
				);

 			ImGui::PopStyleColor(1);
			
			// 現在の子ウィンドウの実スクロール位置を保存
			g_actionLogScrollY = ImGui::GetScrollY();

 			ImGui::EndChild();

			// ---------------------------------------------------
			// スクロールバーが panel の右下角を四角く見せないようにマスク
			// スクロール領域自体の幅・高さは変更せず、角丸の外側だけを
			// 画面背景色で覆って、panel の角丸形状を保つ。
			// ---------------------------------------------------
			{
				// panel の下端が親 window によって切られている場合は、マスクしない。
				if (domain2_bottom_y >= panel_max.y)
				{
					constexpr float CORNER_MASK_STEP = 0.25f;
					const float radius = ACTION_LOG_PANEL_ROUNDING * 1.25f;
					const ImVec2 corner_center(
						panel_max.x - radius,
						panel_max.y - radius
					);

					ImDrawList* foreground_draw_list = ImGui::GetForegroundDrawList();

					for (float y = panel_max.y - radius; y < panel_max.y; y += CORNER_MASK_STEP)
					{
						const float strip_bottom =
							std::min(y + CORNER_MASK_STEP, panel_max.y);
						const float sample_y = (y + strip_bottom) * 0.5f;
						const float dy = sample_y - corner_center.y;
						const float dx = std::sqrt(
							std::max(0.0f, radius * radius - dy * dy)
						);
						const float rounded_edge_x = corner_center.x + dx;

						foreground_draw_list->AddRectFilled(
							ImVec2(rounded_edge_x, y),
							ImVec2(panel_max.x + 1.0f, strip_bottom),
							SIM_BACKGROUND_COLOR
						);
					}
				}
			}


 			ImGui::PopStyleColor(4);

 			ImGui::PopStyleVar(3);
			}
		}
    }

    ImGui::EndChild();


    // =======================================================
    // domain3
    // =======================================================

    ImGui::SetCursorPos(
        ImVec2(
            0.0f,
            split_y
        )
    );

    ImGui::BeginChild(
        "domain3",
        ImVec2(
            DOMAIN3_WIDTH,
            static_cast<float>(WINDOW_HEIGHT) - split_y
        ),
        true,
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse |
        ImGuiWindowFlags_NoBackground
    );

    ImGui::EndChild();


    // =======================================================
    // domain4
    // =======================================================

    ImGui::SameLine(0.0f, 0.0f);

    ImGui::BeginChild(
        "domain4",
        ImVec2(
            DOMAIN4_WIDTH,
            static_cast<float>(WINDOW_HEIGHT) - split_y
        ),
        true,
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse |
        ImGuiWindowFlags_NoBackground
    );

    ImGui::EndChild();

    // =======================================================
    // マウス座標
    // =======================================================

    const ImGuiIO& io = ImGui::GetIO();

    const ImVec2 window_pos =
        ImGui::GetWindowPos();

    const float mouse_x =
        io.MousePos.x - window_pos.x;

    const float mouse_y =
        io.MousePos.y - window_pos.y;


    // =======================================================
    // domain1 / domain2 境界
    //
    // 操作可能
    // =======================================================

    const bool vertical_hover =
        mouse_x >= split_x - SPLITTER_SIZE * 1.5f &&
        mouse_x <= split_x + SPLITTER_SIZE * 1.5f &&
        mouse_y >= 0.0f &&
        mouse_y <= split_y;


    // -------------------------------------------------------
    // ドラッグ開始
    // -------------------------------------------------------

    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) &&
        vertical_hover)
    {
        dragging_vertical = true;
    }


    // -------------------------------------------------------
    // ドラッグ終了
    // -------------------------------------------------------

    if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
    {
        dragging_vertical = false;
    }


    // -------------------------------------------------------
    // ドラッグ中
    // -------------------------------------------------------

    if (dragging_vertical)
    {
        split_x = std::clamp(
            mouse_x,
            MIN_DOMAIN,
            MAX_DOMAIN_X
        );
    }


    // -------------------------------------------------------
    // 右クリックで初期位置へ戻す
    // -------------------------------------------------------

    if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) &&
        vertical_hover)
    {
        split_x = DEFAULT_SPLIT_X;
    }


    // =======================================================
    // domain1,2 / domain3,4 境界
    //
    // 操作可能
    // =======================================================

    const bool horizontal_hover =
        mouse_y >= split_y - SPLITTER_SIZE * 1.5f &&
        mouse_y <= split_y + SPLITTER_SIZE * 1.5f &&
        mouse_x >= 0.0f &&
        mouse_x <= static_cast<float>(WINDOW_WIDTH);


    // -------------------------------------------------------
    // ドラッグ開始
    // -------------------------------------------------------

    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) &&
        horizontal_hover)
    {
        dragging_horizontal = true;
    }


    // -------------------------------------------------------
    // ドラッグ終了
    // -------------------------------------------------------

    if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
    {
        dragging_horizontal = false;
    }


    // -------------------------------------------------------
    // ドラッグ中
    // -------------------------------------------------------

    if (dragging_horizontal)
    {
        split_y = std::clamp(
            mouse_y,
            MIN_DOMAIN,
            MAX_DOMAIN_Y
        );
    }


    // -------------------------------------------------------
    // 右クリックで初期位置へ戻す
    // -------------------------------------------------------

    if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) &&
        horizontal_hover)
    {
        split_y = DEFAULT_SPLIT_Y;
    }


    // =======================================================
    // 境界線描画
    // =======================================================

    ImDrawList* draw_list =
        ImGui::GetForegroundDrawList();


    // -------------------------------------------------------
    // domain3 <-> domain4
    //
    // 操作不能な縦境界（最も奥に描画）
    // -------------------------------------------------------

    draw_list->AddRectFilled(
        ImVec2(
            DOMAIN3_WIDTH - SPLITTER_SIZE * 0.5f,
            split_y
        ),
        ImVec2(
            DOMAIN3_WIDTH + SPLITTER_SIZE * 0.5f,
            static_cast<float>(WINDOW_HEIGHT)
        ),
        INACTIVE_SPLITTER_COLOR
    );


    // -------------------------------------------------------
    // domain1 <-> domain2
    //
    // 操作可能な縦境界
    // -------------------------------------------------------

    const ImU32 vertical_color =
        vertical_hover
            ? ACTIVE_SPLITTER_HOVER_COLOR
            : ACTIVE_SPLITTER_COLOR;

    draw_list->AddRectFilled(
        ImVec2(
            split_x - SPLITTER_SIZE * 0.5f,
            0.0f
        ),
        ImVec2(
            split_x + SPLITTER_SIZE * 0.5f,
            split_y
        ),
        vertical_color
    );


    // -------------------------------------------------------
    // domain1,2 <-> domain3,4
    //
    // 操作可能な横境界
    // -------------------------------------------------------

    const ImU32 horizontal_color =
        horizontal_hover
            ? ACTIVE_SPLITTER_HOVER_COLOR
            : ACTIVE_SPLITTER_COLOR;

    draw_list->AddRectFilled(
        ImVec2(
            0.0f,
            split_y - SPLITTER_SIZE * 0.5f
        ),
        ImVec2(
            static_cast<float>(WINDOW_WIDTH),
            split_y + SPLITTER_SIZE * 0.5f
        ),
        horizontal_color
    );
	
	// =======================================================
	// Back to Formation ボタン
	// =======================================================

	constexpr float BACK_BUTTON_WIDTH    = 180.0f;
	constexpr float BACK_BUTTON_HEIGHT   = 32.0f;
	constexpr float BACK_BUTTON_MARGIN   = 10.0f;
	constexpr float BACK_BUTTON_ROUNDING = 10.0f;

	// domain4 の左上
	const float button_x =
		DOMAIN3_WIDTH + BACK_BUTTON_MARGIN;

	const float button_y =
		split_y + BACK_BUTTON_MARGIN;

	const ImVec2 button_min(
		button_x,
		button_y
	);

	const ImVec2 button_max(
		button_x + BACK_BUTTON_WIDTH,
		button_y + BACK_BUTTON_HEIGHT
	);


	// -------------------------------------------------------
	// クリック判定
	// -------------------------------------------------------

	// domain4 の表示範囲外ではクリック判定を行わない
	const bool within_domain4 =
		mouse_x >= DOMAIN3_WIDTH && mouse_x <= static_cast<float>(WINDOW_WIDTH) &&
		mouse_y >= split_y && mouse_y <= static_cast<float>(WINDOW_HEIGHT);

	const bool back_button_hovered =
		within_domain4 &&
		mouse_x >= button_min.x &&
		mouse_x <= button_max.x &&
		mouse_y >= button_min.y &&
		mouse_y <= button_max.y;

	if (back_button_hovered &&
		ImGui::IsMouseClicked(ImGuiMouseButton_Left))
	{
		next_scene = SceneType::Formation;
	}

	// Escape キーで編成画面に戻る
	if (ImGui::IsKeyPressed(ImGuiKey_Escape))
	{
		next_scene = SceneType::Formation;
	}

	// -------------------------------------------------------
	// ボタン描画
	// -------------------------------------------------------

	const ImU32 back_button_color =
		back_button_hovered
			? IM_COL32(230, 165, 165, 255)
			: IM_COL32(210, 145, 145, 255);

	draw_list->AddRectFilled(
		button_min,
		button_max,
		back_button_color,
		BACK_BUTTON_ROUNDING
	);


	// -------------------------------------------------------
	// テキスト
	// -------------------------------------------------------

	const char* text = "Back to Formation";

	const ImVec2 text_size =
		ImGui::CalcTextSize(text);

	const ImVec2 text_pos(
		button_x +
			(BACK_BUTTON_WIDTH - text_size.x) * 0.5f,

		button_y +
			(BACK_BUTTON_HEIGHT - text_size.y) * 0.5f
	);

	draw_list->AddText(
		text_pos,
		IM_COL32(60, 40, 40, 255),
		text
	);


    // =======================================================
    // マウスカーソル
    // =======================================================

    if (vertical_hover || dragging_vertical)
    {
        ImGui::SetMouseCursor(
            ImGuiMouseCursor_ResizeEW
        );
    }
    else if (horizontal_hover || dragging_horizontal)
    {
        ImGui::SetMouseCursor(
            ImGuiMouseCursor_ResizeNS
        );
    }


    // =======================================================
    // 終了
    // =======================================================

    ImGui::End();

    ImGui::PopStyleVar();

    return next_scene;
}