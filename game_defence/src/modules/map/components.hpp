#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include <vector>

namespace game2d {

// an edge between two cells assumes there's a wall there
// e.g. if there's an edge from (0, 0) to (0, 1)
// the wall goes 90 degrees between them
struct Edge
{
  glm::ivec2 gp_a{ 0, 0 };
  glm::ivec2 gp_b{ 0, 0 };
};

inline bool
operator==(const Edge& a, const Edge& b)
{
  const bool cond_a = a.gp_a == b.gp_a && a.gp_b == b.gp_b;
  const bool cond_b = a.gp_a == b.gp_b && a.gp_b == b.gp_a;
  return cond_a || cond_b;
};

struct MapComponent
{
  int xmax = 10;
  int ymax = 10;
  int tilesize = 50;

  std::vector<std::vector<entt::entity>> map;
};
} // namespace game2d