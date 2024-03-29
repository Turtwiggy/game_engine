#pragma once

#include "entt/entt.hpp"
#include "modules/ai_pathfinding/components.hpp"

#include <vector>

namespace game2d {

struct GridComponent
{
  int size = 32; // worldspace size e.g. pixels
  int width = 10;
  int height = 10;

  // entities on grid
  std::vector<std::vector<entt::entity>> grid;

  // pathfinding
  std::vector<astar_cell> flow_field;
  std::vector<std::vector<entt::entity>> debug_flow_field;

  // generated obstacles
  std::vector<int> generated_obstacles;
  std::vector<entt::entity> debug_generated_obstacles;
};

} // namespace game2d