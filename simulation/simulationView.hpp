/**
 * @file simulationView.hpp
 * @brief 戦闘シミュレーション画面の描画およびUI入力を担当するヘッダーファイル。
 */

#ifndef SIMULATION_VIEW_HPP
#define SIMULATION_VIEW_HPP

#include "../imgui/imgui.h"
#include "../scene_type.hpp"
#include "simulationLogic.hpp"

/**
 * @brief 太字フォントポインタ（main.cppで初期化）。
 */
extern ImFont* g_font_bold;

/**
 * @brief UI側の初期化ラッパー関数。
 * @details UI固有のスクロール状態や入力状態をリセットし、シミュレーションの初期状態を設定します。
 * @param state 設定する初期チーム状態。
 */
void SetSimulationTeamState(const TeamState& state);

/**
 * @brief 戦闘シミュレーション画面を描画し、フレーム更新を行います。
 * @return 遷移先または維持するシーンタイプ。
 */
SceneType RenderSimulationScene();

#endif // SIMULATION_VIEW_HPP
