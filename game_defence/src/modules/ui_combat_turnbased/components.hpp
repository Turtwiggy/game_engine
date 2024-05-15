#pragma once

#include <entt/entt.hpp>

namespace game2d {

struct SINGLE_TurnBasedCombatInfo
{
  entt::entity to_place_debug;
};

// Data transferred from the Duckgame scene to the dungeon scene
struct SINGLE_DuckgameToDungeon
{
  bool backstabbed = false; // was something backstabbed to get here?
};

} // namespace game2d