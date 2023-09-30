#pragma once

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
                       std::vector<std::pair<GridDirection, int>>& results);

struct astar_cell
{
  glm::ivec2 pos;
  int path_cost = 1;
  int distance = INT_MAX;
};

std::vector<astar_cell>
generate_flow_field(const GridComponent& map, const int from_idx)
{
  std::vector<astar_cell> field;
  field.resize(map.width * map.height);

  auto from = field[from_idx];
  from.distance = 0;

  std::vector<astar_cell> frontier;
  frontier.push_back(from);

  // std::vector<entt::entity> ents = map.grid[from_idx];

  while (frontier.size() > 0) {
    astar_cell current = frontier.front();
    frontier.erase(frontier.begin());

    // visible_cell.Add(current);

    std::vector<std::pair<GridDirection, index>> neighbours;
    get_neighbour_indicies(current.pos.x, current.pos.y, grid.width, grid.height, neighbours);

    for (const auto& [dir, idx] : neighbours) {
      auto neighbour = map[idx];

      if (neighbour.cost == -1)
        continue; // skip

      int distance = current.distance + 1;

      if (neighbour.distance == MAX_VALUE) {
        neighbour.distance = distance;
        frontier.push_back(neighbour)
      } else if (distance < neighbour.distance)
        neighbour.distance = distance;
    }
  }
};

} // namespace game2d