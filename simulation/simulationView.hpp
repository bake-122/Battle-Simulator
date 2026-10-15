#ifndef SIMULATION_VIEW_HPP
#define SIMULATION_VIEW_HPP

#include "../imgui/imgui.h"
#include "../scene_type.hpp"
#include "simulationLogic.hpp"

extern ImFont* g_font_bold;  // 太字フォント（main.cppで初期化）

// UI側の初期化ラッパー。UI固有のスクロール状態もリセットする。
void SetSimulationTeamState(const TeamState& state);

SceneType RenderSimulationScene();

#endif // SIMULATION_VIEW_HPP
