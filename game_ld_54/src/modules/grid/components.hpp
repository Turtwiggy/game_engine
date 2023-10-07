#pragma once

#include "entt/entt.hpp"
#include "modules/ai_pathfinding/components.hpp"

#include <vector>

namespace game2d {

struct GridComponent
{
  int size = 64; // worldspace size e.g. pixels
  int width = 10;
  int height = 10;
  std::vector<std::vector<entt::entity>> grid;
  std::vector<astar_cell> flow_field;
  std::vector<std::vector<entt::entity>> debug_flow_field;

  // hmmmmmmmmmmmmmmmm
  std::vector<entt::entity> debug_cell_ents;
  std::vector<int> cell_grid;
};

} // namespace game2d