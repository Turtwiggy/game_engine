#pragma once

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
};

} // namespace game2d