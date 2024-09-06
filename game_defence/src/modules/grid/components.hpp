#pragma once

#include "entt/entt.hpp"
#include "maths/maths.hpp"

#include <functional>
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

inline bool
operator==(const Edge& a, const Edge& b)
{
  const auto eq_a = (a.a_idx == b.a_idx && a.b_idx == b.b_idx);
  const auto eq_b = (a.a_idx == b.b_idx && a.b_idx == b.a_idx);
  return eq_a || eq_b;
};

struct EdgeHash
{
  size_t operator()(const Edge& s) const
  {
    std::size_t h1 = std::hash<int>{}(s.a_idx);
    std::size_t h2 = std::hash<int>{}(s.b_idx);
    return h1 ^ (h2 << 1); // or use boost::hash_combine
  };
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