#pragma once

#include "modules/actor_enemy_patrol/components.hpp"
#include <entt/entt.hpp>

namespace game2d {

struct SINGLE_TurnBasedCombatInfo
{
  entt::entity action_cursor = entt::null;
};

// Data transferred from the Duckgame scene to the dungeon scene
struct OverworldToDungeonInfo
{
  bool backstabbed = false; // was something backstabbed to get here?
  PatrolComponent patrol_that_you_hit;
};

} // namespace game2d