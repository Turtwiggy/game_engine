#pragma once

#include "engine/algorithm_astar_pathfinding/astar_components.hpp"
#include "modules/system_ai/system_ai_components.hpp"
#include <glm/glm.hpp>

#include <vector>

namespace game2d {

struct UIActionState
{
  ActionEnum current = ActionEnum::NONE;
};

struct CompletedActions
{
  std::vector<ActionEnum> actions;
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

struct RequestItem
{
  bool placeholder = true;
};

} // namespace game2d