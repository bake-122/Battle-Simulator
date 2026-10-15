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

struct UnitStatDetails
{
    Character character{};
    int hp = 0;       // 現在HP
    int maxHP = 0;    // 戦闘開始時の最大HP
    int atk = 0;
    int def = 0;
    int currentMP = 0;
    int currentBP = 0; // BPを使用するキオク用。現時点ではゲージ処理未実装。
    int magicLevel = 140;
    float av = 0.0f;
};

struct TeamState
{
    std::array<UnitStatDetails, 5> ally{};
    std::array<UnitStatDetails, 5> enemy{};
    int sp = 5; // パーティ共有SP（初期値5、範囲0～6）
};

struct TimelineNode
{
    int team;       // 0 = enemy, 1 = ally
    int slotIndex;  // 0-4
    int tp;         // enemy: 1-5, ally: 6-10
    int id;
    const char* name;
    int spd;
    float av;
};

enum class SkillType : std::uint8_t
{
    SA = 0,
    BS = 1,
    BA = 2,
    Ability = 3
};


const TeamState& GetSimulationTeamState();
void SetSimulationTeamStateLogic(const TeamState& state);
const std::vector<TimelineNode>& GetTimeline();
const std::vector<std::string>& GetActionLog();
int GetActorTeam();
int GetActorSlot();
float GetAccumTime();
int GetHistoryIndex();
int GetHistoryCount();
SkillType GetCurrentSkillType();
void ToggleCurrentSkillType();

void InitTimeline();
void BackupTurn();
void ForwardTurn();
void RestoreHistoryStep(int index);
void ProceedTurnWithHistory(SkillType skillType, int targetTeam, int targetSlot);


#endif // SIMULATION_LOGIC_HPP
