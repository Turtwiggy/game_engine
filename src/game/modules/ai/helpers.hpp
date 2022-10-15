#pragma once

#include "game/helpers/priority_queue.hpp"
#include "game/modules/dungeon/components.hpp"

#include <entt/entt.hpp>

#include <utility>
#include <vector>

namespace game2d {

struct vec2i
{
  int x = 0;
  int y = 0;

  // spaceship operator
  auto operator<=>(const vec2i&) const = default;
};

// astar pathfinding

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

template<typename T>
std::vector<T>
reconstruct_path(std::map<T, T>& came_from, const T& start, const T& goal)
{
  T current = goal;

  std::vector<T> path;

  while (!equal<T>(current, start)) {
    path.push_back(current);

    T copy = current;
    current = came_from[current];

    if (equal<T>(copy, current)) {
      printf("pathfinding hit a loop?");
      break;
    }
  }

  path.push_back(start); // optional to add end
  std::reverse(path.begin(), path.end());
  return path;
}

template<typename T>
void
evaluate_neighbour(std::map<T, T>& came_from,
                   std::map<T, int>& cost_so_far,
                   PriorityQueue<T>& frontier,
                   const T& current,
                   const T& neighbour,
                   const int neighbour_cost,
                   const T& to)
{
  // Assumption:
  // assume multiple entities on the same tile
  // are the same path cost.
  int map_cost = neighbour_cost;
  if (map_cost == -1)
    return; // impassable
  int new_cost = cost_so_far[current] + map_cost;
  if (!cost_so_far.contains(neighbour) || new_cost < cost_so_far[neighbour]) {
    cost_so_far[neighbour] = new_cost;
    int priority = new_cost + heuristic<T>(neighbour, to);
    frontier.enqueue(neighbour, priority);
    came_from[neighbour] = current;
  }
}

std::vector<vec2i>
astar(entt::registry& registry, const vec2i& from, const vec2i& to);

// e.g. 0, 0 should return {1, 0}, {1, 1}, {0, 1}
void
get_neighbour_indicies(const int x,
                       const int y,
                       const int x_max,
                       const int y_max,
                       std::vector<std::pair<GridDirection, int>>& results);

} // namespace game2d