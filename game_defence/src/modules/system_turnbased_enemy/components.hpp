#pragma once

#include "modules/combat_damage/components.hpp"
#include <entt/entt.hpp>

namespace game2d {

// per-enemy info...
struct TurnState
{
  bool has_moved = false;
  bool has_shot = false;

  // hack for debugging enemy ai
  bool do_move = true;
  bool do_shoot = true;
};

// when the team's turn is complete...
struct RequestToCompleteTurn
{
  AvailableTeams team = AvailableTeams::neutral;
};

// scene data...
struct SINGLE_CombatState
{
  AvailableTeams team = AvailableTeams::neutral;
};

struct TurnBasedUnitComponent
{
  bool placeholder = true;
};

} // namespace game2d