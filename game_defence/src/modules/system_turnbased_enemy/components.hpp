#pragma once

#include "helpers/entity_pool.hpp"
#include "modules/combat_damage/components.hpp"
#include <entt/entt.hpp>

namespace game2d {

enum class Actions
{
  MOVE,
  ATTACK,
};

// Each entity gets X actions before the other team gets to go
struct ActionState
{
  int actions_available = 2; // per turn
  int actions_completed = 0;
  // std::vector<> actions_completed;
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

  EntityPool show_selected_player_path;
};

} // namespace game2d