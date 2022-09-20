#include "ai.hpp"

#include "engine/maths/grid.hpp"
#include "engine/maths/maths.hpp"

#include "game/components/components.hpp"
#include "game/helpers/dungeon.hpp"
#include "game/helpers/priority_queue.hpp"
#include "modules/events/helpers/mouse.hpp"
#include "modules/physics/components.hpp"

#include <glm/glm.hpp>
#include <unordered_map>
#include <vector>

namespace game2d {

// astar pathfinding

template<class T>
int
distance(const T& a, const T& b)
{
  int distance_x = a.x < b.x ? b.x - a.x : a.x - b.x;
  int distance_y = a.y < b.y ? b.y - a.y : a.y - b.y;
  return (distance_x + distance_y) / 2;
}

template<class T>
int
astar_heuristic(const T& a, const T& b)
{
  return distance<T>(a, b);
};

template<class T>
std::vector<T>
reconstruct_path(std::unordered_map<T, T>& came_from, const T& start, const T& goal)
{
  T current = goal;

  std::vector<T> path;

  while (current != start) {
    path.push_back(current);
    current = came_from[current];
  }

  // optional
  path.push_back(start);
  std::reverse(path.begin(), path.end());
  return path;
}

template<class T>
std::vector<T>
astar(const T& from, const T& to)
{
  if (from == to || distance(from, to) == 0)
    return {};
  PriorityQueue<T> frontier;
  frontier.enqueue(from, 0);
  std::unordered_map<T, T> came_from;
  std::unordered_map<T, int> cost_so_far;
  came_from[from] = from;
  cost_so_far[from] = 0;

  while (frontier.size()) {
    T current = frontier.dequeue();
    if (current == to)
      return reconstruct_path<T>(came_from, from, to);

    // four corners, cuz" we're in a square grid
    for (int i = 0; i < static_cast<int>(GridDirection::COUNT); i++) {
      // T neighbour = get_neighbour(i);

      // TODO: carry on from here
    }

    break;
  }

  return {};
}

void
init_ai_system(entt::registry& r){
  //
};

void
update_ai_system(entt::registry& r)
{
  // std::vector<std::reference_wrapper<GridTileComponent>> map;
  // {
  //   const auto& view = r.view<GridTileComponent>();
  //   for (const auto& [entity, grid] : view.each()) {
  //     map.push_back(grid);
  //   }
  // }

  // const auto& player_view = r.view<PlayerComponent>();
  // const auto player_entity = player_view.front();
  // const auto& player_transform = r.get<TransformComponent>(player_entity);
  // glm::ivec2 player_grid_pos =
  //   engine::grid::world_space_to_grid_space({ player_transform.position.x, player_transform.position.y }, 16);

  const int GRID_SIZE = 16;
  const glm::ivec2 mouse_position = mouse_position_in_worldspace(r);
  const glm::ivec2 world_position = engine::grid::world_space_to_clamped_world_space(mouse_position, GRID_SIZE);

  const auto& view = r.view<const AiBrainComponent, GridTileComponent, GridMoveComponent>();
  for (auto [entity, ai, grid, move] : view.each()) {
    const glm::ivec2 current_position = { grid.x, grid.y };
    //   // const glm::ivec2 desired_position = ; //
    //   // std::vector<glm::ivec2> path = pathfind(current_position, desired_position);
    // glm::ivec2 dir = current_position - grid_position;

    // move in desired direction (with cooldown)
    // move.x -= glm::clamp(static_cast<int>(dir.x), -1, 1);
    // move.y -= glm::clamp(static_cast<int>(dir.y), -1, 1);
  }
};

} // namespace game2d
