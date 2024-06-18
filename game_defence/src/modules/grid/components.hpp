#pragma once

#include "entt/entt.hpp"
#include "modules/algorithm_astar_pathfinding/components.hpp"

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

struct MapComponent
{
  int xmax = 10;
  int ymax = 10;
  int tilesize = 50;

  std::vector<std::vector<entt::entity>> map;
  std::vector<Edge> edges;
};

} // namespace game2d