#pragma once

#include "engine/algorithm_astar_pathfinding/astar_components.hpp"
#include <glm/glm.hpp>

#include <vector>

namespace game2d {

struct UIActionState
{
  // ActionEnum current = ActionEnum::NONE;
  std::string current = "none";
};

struct CompletedActions
{
  // std::vector<ActionEnum> actions;
  std::vector<std::string> actions;
};

struct RequestEndTurn
{
  bool placeholder = true;
};

struct RequestMove
{
  GeneratedPathComponent path_c;
};

struct RequestAttack
{
  std::vector<entt::entity> targets;
};

struct RequestHeal
{
  int amount = 0;
};

struct RequestItem
{
  bool placeholder = true;
};

//
// Events
//

struct EndTurnEvent
{
  entt::entity e = entt::null;
};

} // namespace game2d