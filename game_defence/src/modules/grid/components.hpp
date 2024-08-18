#pragma once

#include "entt/entt.hpp"

#include <vector>

namespace game2d {

// an edge between two cells assumes there's a wall there
// e.g. if there's an edge from (0, 0) to (0, 1)
// the wall goes 90 degrees between them
struct Edge
{
  int a_idx = 0;
  int b_idx = 0;
  entt::entity instance = entt::null;
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