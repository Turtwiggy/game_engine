#pragma once

#include <entt/entt.hpp>

namespace game2d {

struct SINGLE_TurnBasedCombatInfo
{
  entt::entity action_cursor = entt::null;
};

// Data transferred from the Duckgame scene to the dungeon scene
// this should be things like; the crew you chose, their items. etc

struct OverworldToDungeonInfo
{
  bool placeholder = true;
};

} // namespace game2d