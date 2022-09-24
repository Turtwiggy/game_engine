#include "ai.hpp"

#include "engine/maths/grid.hpp"
#include "engine/maths/maths.hpp"

#include "game/components/components.hpp"
#include "game/helpers/dungeon.hpp"
#include "game/helpers/priority_queue.hpp"
#include "modules/events/helpers/mouse.hpp"
#include "modules/physics/components.hpp"
#include "resources/colour.hpp"

#include <glm/glm.hpp>

#include <functional>
#include <map>
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
}

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
astar(entt::registry& registry, const vec2i& from, const vec2i& to)
{
  // hack: only one dungeon at the moment
  const auto d = registry.view<Dungeon>().front();
  const auto& dungeon = registry.get<Dungeon>(d);
  const int x_max = dungeon.width;

  if (equal<vec2i>(from, to)) //|| distance(from, to) == 0)
    return {};

  PriorityQueue<vec2i> frontier;
  frontier.enqueue(from, 0);
  std::map<vec2i, vec2i> came_from;
  std::map<vec2i, int> cost_so_far;
  came_from[from] = from;
  cost_so_far[from] = 0;

  // read-only view of the grid
  const auto& group = registry.group<GridComponent, PathfindableComponent>();
  group.sort<GridComponent>([&x_max](const auto& a, const auto& b) {
    int index_a = x_max * a.y + a.x;
    int index_b = x_max * b.y + b.x;
    return index_a < index_b;
  });

  while (frontier.size() > 0) {
    vec2i current = frontier.dequeue();

    if (equal<vec2i>(current, to))
      return reconstruct_path<vec2i>(came_from, from, to);

    std::vector<std::pair<GridDirection, int>> results;
    get_neighbour_indicies(current.x, current.y, dungeon.width, dungeon.height, results);

    for (const auto& neighbour_idx : results) {
      const auto& entity = group[neighbour_idx.second];
      const auto [grid, path] = group.get<GridComponent, PathfindableComponent>(entity);

      const vec2i neighbour = { grid.x, grid.y };
      const int neighbour_cost = path.cost;
      evaluate_neighbour<vec2i>(came_from, cost_so_far, frontier, current, neighbour, neighbour_cost, to);
    }
  }

  return {};
}

void
update_ai_system(entt::registry& r, const uint64_t& milliseconds_dt)
{
  auto& colours = r.ctx().at<SINGLETON_ColoursComponent>();

  // hack: only one dungeon at the moment
  const auto dungeon = r.view<Dungeon>().front();
  const auto& d = r.get<Dungeon>(dungeon);
  const int& x_max = d.width;

  const int GRID_SIZE = 16;
  const auto& player_view = r.view<PlayerComponent>();
  const auto player_entity = player_view.front();
  const auto& player_transform = r.get<TransformComponent>(player_entity);
  const auto grid_position =
    engine::grid::world_space_to_grid_space({ player_transform.position.x, player_transform.position.y }, GRID_SIZE);

  const glm::ivec2 mouse_position = mouse_position_in_worldspace(r) + glm::ivec2(GRID_SIZE / 2, GRID_SIZE / 2);
  const glm::ivec2 mouse_grid_position = engine::grid::world_space_to_grid_space(mouse_position, GRID_SIZE);

  // mouse location
  vec2i to = { glm::clamp(mouse_grid_position.x, 0, d.width - 1), glm::clamp(mouse_grid_position.y, 0, d.height - 1) };

  // Temporary optmisation:
  // if hovering over a WALL tile,
  // skip all pathfinding
  auto hovered = grid_entities_at(r, to.x, to.y);
  bool skip_all_pathfinding = false;
  for (auto& e : hovered) {
    EntityTypeComponent t = r.get<EntityTypeComponent>(e);
    if (t.type == EntityType::wall)
      skip_all_pathfinding = true;
  }
  if (skip_all_pathfinding)
    return;

  const auto& group = r.group<GridComponent, PathfindableComponent>();
  group.sort<GridComponent>([&x_max](const auto& a, const auto& b) {
    int index_a = x_max * a.y + a.x;
    int index_b = x_max * b.y + b.x;
    return index_a < index_b;
  });

  const auto& view = r.view<AiBrainComponent, GridComponent, GridMoveComponent>();
  for (auto [entity, ai, grid, move] : view.each()) {

    vec2i from;
    from.x = glm::clamp(grid.x, 0, d.width - 1);
    from.y = glm::clamp(grid.y, 0, d.height - 1);

    const auto path = astar(r, from, to);
    for (const auto& p : path) {
      int index = x_max * p.y + p.x;
      const auto& e = group[index];
      auto& col = r.get<SpriteColourComponent>(e);
      col.colour = colours.lin_cyan;
    }
  }

  //   // move in desired direction (with cooldown)
  //   // move.x -= glm::clamp(static_cast<int>(dir.x), -1, 1);
  //   // move.y -= glm::clamp(static_cast<int>(dir.y), -1, 1);
  // }

  //
};

} // namespace game2d
