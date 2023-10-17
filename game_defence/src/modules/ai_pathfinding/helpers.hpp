#pragma once

#include "maths/grid.hpp"
#include "modules/ai_pathfinding/components.hpp"
#include "modules/grid/components.hpp"

#include <glm/glm.hpp>

#include <utility>
#include <vector>

namespace game2d {

template<typename T>
int
distance(const T& a, const T& b)
{
  int distance_x = a.x < b.x ? b.x - a.x : a.x - b.x;
  int distance_y = a.y < b.y ? b.y - a.y : a.y - b.y;
  return (distance_x + distance_y);
};

template<typename T>
bool
equal(const T& a, const T& b)
{
  return (a.x == b.x) && (a.y == b.y);
};

template<typename T>
int
heuristic(const T& a, const T& b)
{
  return distance<T>(a, b);
};

// e.g. 0, 0 should return {1, 0}, {1, 1}, {0, 1}
void
get_neighbour_indicies(const int x,
                       const int y,
                       const int x_max,
                       const int y_max,
                       std::vector<std::pair<engine::grid::GridDirection, int>>& results);

[[nodiscard]] std::vector<glm::ivec2>
generate_direct(entt::registry& r, const GridComponent& grid, const int from_idx, const int to_idx);

[[nodiscard]] std::vector<astar_cell>
generate_flow_field(entt::registry& r, const GridComponent& grid, const int from_idx);

void
display_flow_field_with_visuals(entt::registry& r, GridComponent& grid);

} // namespace game2d