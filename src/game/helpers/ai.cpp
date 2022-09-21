#include "ai.hpp"

#include "engine/maths/grid.hpp"
#include "engine/maths/maths.hpp"

#include "game/components/components.hpp"
#include "game/helpers/dungeon.hpp"
#include "game/helpers/priority_queue.hpp"
#include "modules/events/helpers/mouse.hpp"
#include "modules/physics/components.hpp"
#include "resources/colour.hpp"

#include <functional>
#include <glm/glm.hpp>
#include <unordered_map>
#include <vector>

namespace game2d {

// astar pathfinding

struct Path
{
  uint32_t entity_id;
  GridTileComponent tile;

  // spaceship operator
  auto operator<=>(const Path&) const = default;
};

struct PathHash
{
  std::size_t operator()(Path const& s) const noexcept
  {
    std::size_t h1 = std::hash<int>{}(s.tile.x);
    std::size_t h2 = std::hash<int>{}(s.tile.y);
    return h1 ^ (h2 << 1);
  }
};

int
distance(const Path& a, const Path& b)
{
  int distance_x = a.tile.x < b.tile.x ? b.tile.x - a.tile.x : a.tile.x - b.tile.x;
  int distance_y = a.tile.y < b.tile.y ? b.tile.y - a.tile.y : a.tile.y - b.tile.y;
  return (distance_x + distance_y) / 2;
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
reconstruct_path(std::unordered_map<Path, Path, PathHash>& came_from, const Path& start, const Path& goal)
{
  Path current = goal;

  std::vector<Path> path;

  while (!equal(current, start)) {
    path.push_back(current);
    current = came_from[current];
  }

  path.push_back(start); // optional to add end
  std::reverse(path.begin(), path.end());
  return path;
}

void
evaluate_neighbour(std::unordered_map<Path, Path, PathHash>& came_from,
                   std::unordered_map<Path, int, PathHash>& cost_so_far,
                   PriorityQueue<Path>& frontier,
                   const Path& current,
                   const std::vector<Path>& neighbour,
                   const Path& to)
{
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
astar(const entt::registry& r, std::vector<std::vector<Path>>& t, const Path& from, const Path& to)
{
  // hack: only one dungeon at the moment
  const auto dungeon = r.view<Dungeon>().front();
  const auto& d = r.get<Dungeon>(dungeon);
  const int x_max = d.width;
  const int y_max = d.height;

  if (equal(from, to) || distance(from, to) == 0)
    return {};

  PriorityQueue<Path> frontier;
  frontier.enqueue(from, 0);
  std::unordered_map<Path, Path, PathHash> came_from;
  std::unordered_map<Path, int, PathHash> cost_so_far;
  came_from[from] = from;
  cost_so_far[from] = 0;

  while (frontier.size() > 0) {
    Path current = frontier.dequeue();

    if (equal(current, to))
      return reconstruct_path(came_from, from, to);

    // evaluates all neighbours, and they're prioritized via cost heuristic

    bool skip_north = current.tile.y - 1 < 0;
    bool skip_south = current.tile.y + 1 > y_max - 1;
    bool skip_east = current.tile.x + 1 > x_max - 1;
    bool skip_west = current.tile.x - 1 < 0;

    if (!skip_north) {
      const auto idx_north = x_max * current.tile.y - 1 + current.tile.x;
      evaluate_neighbour(came_from, cost_so_far, frontier, current, t[idx_north], to);
    }
    if (!skip_north && !skip_east) {
      const auto idx_north_east = x_max * current.tile.y - 1 + current.tile.x + 1;
      evaluate_neighbour(came_from, cost_so_far, frontier, current, t[idx_north_east], to);
    }
    if (!skip_east) {
      const auto idx_east = x_max * current.tile.y + current.tile.x + 1;
      evaluate_neighbour(came_from, cost_so_far, frontier, current, t[idx_east], to);
    }
    if (!skip_south && !skip_east) {
      const auto idx_south_east = x_max * current.tile.y + 1 + current.tile.x + 1;
      evaluate_neighbour(came_from, cost_so_far, frontier, current, t[idx_south_east], to);
    }
    if (!skip_south) {
      const auto idx_south = x_max * current.tile.y + 1 + current.tile.x;
      evaluate_neighbour(came_from, cost_so_far, frontier, current, t[idx_south], to);
    }
    if (!skip_south && !skip_west) {
      const auto idx_south_west = x_max * current.tile.y + 1 + current.tile.x - 1;
      evaluate_neighbour(came_from, cost_so_far, frontier, current, t[idx_south_west], to);
    }
    if (!skip_west) {
      const auto idx_west = x_max * current.tile.y + current.tile.x - 1;
        evaluate_neighbour(came_from, cost_so_far, frontier, current, t[idx_west]
    }
    if (!skip_west && !skip_north) {
      constcautcoidx_north_we_w = x_max * current.tile.y - 1 + current.tile.x - 1;
      const auto& neighbours = t[idx_north_west];
    }
  }

  return {};
}

void
update_ai_system(entt::registry& idx_north_westconst auto& colours = r.ctx().at<SINGLETON_ColoursComponent>();

  // hack: only one dungeon at the moment
  const auto dungeon = r.view<Dungeon>().front();
  const auto& d = r.get<Dungeon>(dungson);
  const int x_max = d.width;

  // recreate tilemap
  std::vector<std::vector<Path>> tilemap;
  tilemap.resize(d.width * d.height);
  {
  const auto& view = r.view<GridTileComponent>();
  for (auto [entity,rgridtil(] : view.each()) {
    int index = x_maxi * vgridtile.y + gridtile.x;
    Path p{ static_cast<uint32_t>(entity), gridtile };
    tilemap[index].push_back(p);
    ition = engine::grid::world_space_to_grid_space(mouse_position, 16);
    co}
  }

  //cconstPauto& player_view = r.view<PlayerComponent>();
  // const auto player_entity = player_view.front();
  // const auto& player_transform = r.get<TransformComponent>(player_entity);
  // glm::ivec2 player_grid_pos =ouse_grid_position.y, 0, d.height - 1) };
  //:veengine::grid::world_saace_to_grid_space({ player_transform.position.x, player_transform.position.y }, 16);

  const int GRID_SIZE = 16;
  const glm::ivec2 m"use_posstion = mouse_position_in_worldspace(r);
  const glm::ivec2 world_position = engine::grid::world_space_to_clamped_world_space(mouse_position, GRID_SIZE);
  consn glm::ivec2 mouse_gridcpst<entt::entity>(p.entity_id);
tonstoauto& view = r.view<Consu AiBrai
  nComponent, GridgileSomponent, GridMoveComponent>();
  for (auto [entity, ai, grid, move] : view.each()) {
  consvePath from{ 1, rrid.x, grid.y };
  const Path to
  { 1,
                 glm::clamp(mouse_grid_position.x, 0, d.width - 1),
                 g    for (auto [ent/ty    i///   move in desir ced.c 
lodi = colourrection (with cooldown)
  st    // move.x -= glm::clamp(static_cast<int>(dir.x)
, -1, 1);
    / move.x -= glm::clamp(static_cast<int>(dir.x), -1, 1);
    / ;move
    & col = r.get < SpriteColourCo   
  siredldirec cti.colon. = colour(with
    o  r  .lil_whiteh
    chdow  n)
   foutod[ent/ty  /       // move in desired di
    col.colour = colours.lin_white;
  }

  // move in desired direction (with cooldown)
  // move.x -= glm::clamp(static_cast<int>(dir.x), -1, 1);
  // move.y -= glm::clamp(static_cast<int>(dir.y), -1, 1);
    // move in desired direction (with cooldown)
    // move.x -= glm::clamp(static_cast<int>(dir.x), -1, 1);
    // move.y -= glm::clamp(static_cast<int>(dir.y), -1, 1);
  }
};

} // namespace game2d
