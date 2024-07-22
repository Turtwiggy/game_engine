#pragma once

#include "entt/entt.hpp"
#include "modules/algorithm_astar_pathfinding/components.hpp"

#include <vector>

namespace game2d {

struct MapComponent
{
  int xmax = 10;
  int ymax = 10;
  int tilesize = 50;

  std::vector<std::vector<entt::entity>> map;
  std::vector<Edge> edges;
};

} // namespace game2d