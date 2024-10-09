#pragma once

#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include <vector>

namespace game2d {

inline auto
ivec2_less(const glm::ivec2& v1, const glm::ivec2& v2)
{
  if (v1.x != v2.x)
    return v1.x < v2.x; // compare by x
  return v1.y < v2.y;   // if x is the same, compare by y
};

// an edge between two cells assumes there's a wall there
// e.g. if there's an edge from (0, 0) to (0, 1)
// the wall goes 90 degrees between them
struct Edge
{
  glm::ivec2 a{ 0, 0 }; // grid pos
  glm::ivec2 b{ 0, 0 }; // grid pos

  Edge() = default;
  Edge(const glm::ivec2 in_a, glm::ivec2 in_b)
  {
    a = in_a;
    b = in_b;
    if (ivec2_less(b, a))
      std::swap(a, b);
  };
};

inline bool
operator==(const Edge& a, const Edge& b)
{
  const bool cond_a = a.a == b.a && a.b == b.b;
  const bool cond_b = a.a == b.b && a.b == b.a;
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