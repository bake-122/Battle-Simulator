/**
 * @file unity.cpp
 * @brief Releaseビルド用の単一コンパイル単位（Unity Build）ファイル
 */

// ==========================================
// Unity Build
// ==========================================

// ImGui core
#include "imgui/imgui.cpp"
#include "imgui/imgui_draw.cpp"
#include "imgui/imgui_tables.cpp"
#include "imgui/imgui_widgets.cpp"

// Application
#include "image_loader.cpp"
#include "formation/formationView.cpp"
#include "simulation/simulationView.cpp"
#include "simulation/simulationLogic.cpp"
#include "simulation/EffectList.cpp"
#include "simulation/ProcessList.cpp"
#include "main.cpp"