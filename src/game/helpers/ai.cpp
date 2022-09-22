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
#include <vector>

namespace game2d {

// astar pathfinding

struct Path
{
  entt::entity entity = entt::null;
  GridTileComponent tile;

  // spaceship operator
  auto operator<=>(const Path&) const = default;
};

int
distance(const Path& a, const Path& b)
{
  int distance_x = a.tile.x < b.tile.x ? b.tile.x - a.tile.x : a.tile.x - b.tile.x;
  int distance_y = a.tile.y < b.tile.y ? b.tile.y - a.tile.y : a.tile.y - b.tile.y;
  return (distance_x + distance_y);
}

int
heuristic(const Path& a, const Path& b)
{
  return distance(a, b);
};

bool
equal(const Path& a, const Path& b)
{
  return (a.tile.x == b.tile.x) && (a.tile.y == b.tile.y);
};

std::vector<Path>
reconstruct_path(std::map<Path, Path>& came_from, const Path& start, const Path& goal)
{
  Path current = goal;

  std::vector<Path> path;

  while (!equal(current, start)) {
    path.push_back(current);

    Path copy = current;
    current = came_from[current];

    if (equal(copy, current)) {
      printf("we got a problem");
      break;
    }
  }

  path.push_back(start); // optional to add end
  std::reverse(path.begin(), path.end());
  return path;
}

void
evaluate_neighbour(std::map<Path, Path>& came_from,
                   std::map<Path, int>& cost_so_far,
                   PriorityQueue<Path>& frontier,
                   const Path& current,
                   const Path& neighbour,
                   const Path& to)
{
  // Assumption:
  // assume multiple entities on the same tile
  // are the same path cost.
  int map_cost = neighbour.tile.cost;
  int new_cost = cost_so_far[current] + map_cost;
  if (!cost_so_far.contains(neighbour) || new_cost < cost_so_far[neighbour]) {
    cost_so_far[neighbour] = new_cost;
    int priority = new_cost + heuristic(neighbour, to);
    frontier.enqueue(neighbour, priority);
    came_from[neighbour] = current;
  }
}

std::vector<Path>
astar(const entt::registry& r, std::vector<Path>& t, const Path& from, const Path& to)
{
  // hack: only one dungeon at the moment
  const auto dungeon = r.view<Dungeon>().front();
  const auto& d = r.get<Dungeon>(dungeon);
  const int x_max = d.width;
  const int y_max = d.height;

  if (equal(from, to)) //|| distance(from, to) == 0)
    return {};

  PriorityQueue<Path> frontier;
  frontier.enqueue(from, 0);
  std::map<Path, Path> came_from;
  std::map<Path, int> cost_so_far;
  came_from[from] = from;
  cost_so_far[from] = 0;

  while (frontier.size() > 0) {
    Path current = frontier.dequeue();

    if (equal(current, to))
      return reconstruct_path(came_from, from, to);

    // evaluates all neighbours, and they're prioritized via cost heuristic

    const int idx_north = x_max * (current.tile.y - 1) + (current.tile.x);
    const int idx_east = x_max * (current.tile.y) + (current.tile.x + 1);
    const int idx_south = x_max * (current.tile.y + 1) + (current.tile.x);
    const int idx_west = x_max * (current.tile.y) + (current.tile.x - 1);
    const int idx_north_east = x_max * (current.tile.y - 1) + (current.tile.x + 1);
    const int idx_south_east = x_max * (current.tile.y + 1) + (current.tile.x + 1);
    const int idx_south_west = x_max * (current.tile.y + 1) + (current.tile.x - 1);
    const int idx_north_west = x_max * (current.tile.y - 1) + (current.tile.x - 1);
    const int max_idx = d.width * d.height;

    // borders
    const bool ignore_north = current.tile.y == 0;
    const bool ignore_east = current.tile.x == x_max;
    const bool ignore_south = current.tile.y == y_max;
    const bool ignore_west = current.tile.x == 0;
    const bool ignore_north_east = ignore_north | ignore_east;
    const bool ignore_south_east = ignore_south | ignore_east;
    const bool ignore_south_west = ignore_south | ignore_west;
    const bool ignore_north_west = ignore_north | ignore_west;

    if (!ignore_north && idx_north >= 0 && idx_north < max_idx)
      evaluate_neighbour(came_from, cost_so_far, frontier, current, t[idx_north], to);

    if (!ignore_north_east && idx_north_east >= 0 && idx_north_east < max_idx)
      evaluate_neighbour(came_from, cost_so_far, frontier, current, t[idx_north_east], to);

    if (!ignore_east && idx_east >= 0 && idx_east < max_idx)
      evaluate_neighbour(came_from, cost_so_far, frontier, current, t[idx_east], to);

    if (!ignore_south_east && idx_south_east >= 0 && idx_south_east < max_idx)
      evaluate_neighbour(came_from, cost_so_far, frontier, current, t[idx_south_east], to);

    if (!ignore_south && idx_south >= 0 && idx_south < max_idx)
      evaluate_neighbour(came_from, cost_so_far, frontier, current, t[idx_south], to);

    if (!ignore_south_west && idx_south_west >= 0 && idx_south_west < max_idx)
      evaluate_neighbour(came_from, cost_so_far, frontier, current, t[idx_south_west], to);

    if (!ignore_west && idx_west >= 0 && idx_west < max_idx)
      evaluate_neighbour(came_from, cost_so_far, frontier, current, t[idx_west], to);

    if (!ignore_north_west && idx_north_west >= 0 && idx_north_west < max_idx)
      evaluate_neighbour(came_from, cost_so_far, frontier, current, t[idx_north_west], to);
  }

  return {};
}

//
//
//

void
update_ai_system(entt::registry& r)
{
  auto& colours = r.ctx().at<SINGLETON_ColoursComponent>();

  // hack: only one dungeon at the moment
  const auto dungeon = r.view<Dungeon>().front();
  const auto& d = r.get<Dungeon>(dungeon);
  const int x_max = d.width;

  // recreate tilemap
  std::vector<Path> tilemap;
  tilemap.resize(d.width * d.height);
  {
    const auto& view = r.view<GridTileComponent, SpriteColourComponent>();
    for (auto [entity, grid, col] : view.each()) {
      int index = (x_max * grid.y) + grid.x;

      auto what_is_this = tilemap[index];
      if (what_is_this.entity != entt::null)
        printf("warning; overwriting entity in tilemap");

      Path tile;
      tile.entity = entity;
      tile.tile = grid;
      tilemap[index] = tile;
    }
  }

  const int GRID_SIZE = 16;
  const auto& player_view = r.view<PlayerComponent>();
  const auto player_entity = player_view.front();
  const auto& player_transform = r.get<TransformComponent>(player_entity);
  const auto grid_position =
    engine::grid::world_space_to_grid_space({ player_transform.position.x, player_transform.position.y }, GRID_SIZE);

  const glm::ivec2 mouse_position = mouse_position_in_worldspace(r) - glm::ivec2{ GRID_SIZE / 2, GRID_SIZE / 2 };
  // const glm::ivec2 world_position = engine::grid::world_space_to_clamped_world_space(mouse_position, GRID_SIZE);
  const glm::ivec2 mouse_grid_position = engine::grid::world_space_to_grid_space(mouse_position, GRID_SIZE);

  Path from;
  from.tile.x = glm::clamp(grid_position.x, 0, d.width - 1);
  from.tile.y = glm::clamp(grid_position.y, 0, d.height - 1);
  from.tile.cost = 1;
  const auto from_entities = grid_entities_at(r, from.tile.x, from.tile.y);
  from.entity = from_entities[0];
  from.tile.cost = r.get<GridTileComponent>(from_entities[0]).cost;

  Path to;
  to.tile.x = glm::clamp(mouse_grid_position.x, 0, d.width - 1);
  to.tile.y = glm::clamp(mouse_grid_position.y, 0, d.height - 1);
  const auto to_entities = grid_entities_at(r, to.tile.x, to.tile.y);
  to.entity = to_entities[0];
  to.tile.cost = r.get<GridTileComponent>(to_entities[0]).cost;

  const auto path = astar(r, tilemap, from, to);
  for (const auto& p : path) {
    auto& col = r.get<SpriteColourComponent>(p.entity);
    col.colour = colours.lin_cyan;
  }

  // const auto& view = r.view<AiBrainComponent, GridTileComponent, GridMoveComponent>();
  // for (auto [entity, ai, grid, move] : view.each())
  // move in desired direction (with cooldown)
  // move.x -= glm::clamp(static_cast<int>(dir.x), -1, 1);
  // move.y -= glm::clamp(static_cast<int>(dir.y), -1, 1);
};

} // namespace game2d
