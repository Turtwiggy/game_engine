#include "helpers.hpp"

#include "actors.hpp"
#include "entt/helpers.hpp"
#include "maths/grid.hpp"
#include "modules/actors/helpers.hpp"
#include "modules/algorithm_astar_pathfinding/components.hpp"
#include "modules/algorithm_astar_pathfinding/priority_queue.hpp"
#include "modules/combat_flash_on_damage/helpers.hpp"
#include "modules/grid/components.hpp"
#include "sprites/helpers.hpp"

#include <map>

namespace game2d {

// Converts GridComponent to std::vector<astar_cell>
std::vector<astar_cell>
generate_map_view(entt::registry& r, const GridComponent& grid)
{
  std::vector<astar_cell> result;

  for (int xy = 0; xy < grid.width * grid.height; xy++) {
    const auto gpos = engine::grid::index_to_grid_position(xy, grid.width, grid.height);

    astar_cell cell;
    cell.idx = xy;
    cell.pos = { gpos.x, gpos.y };

    // If the cell contains entites with cost info,
    // add that cost info to the cost
    for (const auto& ent : grid.grid[xy]) {
      if (auto* cost = r.try_get<PathfindComponent>(ent))
        cell.cost = cost->cost;
    }

    result.push_back(cell);
  }

  // std::cout << "map size: " << result.size() << std::endl;
  return result;
}

std::vector<glm::ivec2>
reconstruct_path(std::map<vec2i, vec2i> came_from, vec2i from, vec2i to)
{
  auto current = to;

  std::vector<glm::ivec2> path;

  while (!equal<vec2i>(current, from)) {
    path.push_back({ current.x, current.y });

    auto copy = current;
    current = came_from[current];

    if (equal<vec2i>(copy, current)) {
      fprintf(stderr, "(error) pathfinding hit a loop.");
      break;
    }
  }

  path.push_back({ from.x, from.y }); // optional to add end
  std::reverse(path.begin(), path.end());
  return path;
}

std::vector<glm::ivec2>
generate_direct(entt::registry& r,
                const GridComponent& grid,
                const int from_idx,
                const int to_idx,
                const std::optional<std::vector<Edge>> edges)
{
  std::vector<astar_cell> map = generate_map_view(r, grid);

  std::vector<vec2i> path;

  const auto from_glm = engine::grid::index_to_grid_position(from_idx, grid.width, grid.height);
  const auto to_glm = engine::grid::index_to_grid_position(to_idx, grid.width, grid.height);
  const vec2i from{ from_glm.x, from_glm.y };
  const vec2i to{ to_glm.x, to_glm.y };

  if (equal<vec2i>(from, to))
    return {};

  PriorityQueue<vec2i> frontier;
  frontier.enqueue(from, 0);
  std::map<vec2i, vec2i> came_from;
  std::map<vec2i, int> cost_so_far;
  came_from[from] = from;
  cost_so_far[from] = 0;

  while (frontier.size() > 0) {
    const auto current = frontier.dequeue();

    if (equal<vec2i>(current, to))
      return reconstruct_path(came_from, from, to);

    const auto neighbour_idxs = engine::grid::get_neighbour_indicies(current.x, current.y, grid.width, grid.height);

    for (const auto& [dir, idx] : neighbour_idxs) {
      const auto& neighbour = map[idx].pos;
      const auto& neighbour_cost = map[idx].cost;

      // Check the edges
      if (edges.has_value()) {
        // Check if there's an edge between the current cell and the neighbour cell.
        // If there is, skip evaluating this neighbour.

        const std::vector<Edge>& es = edges.value();

        bool wall_between_grid = false;

        for (const Edge& e : es) {
          wall_between_grid |= e.cell_a == current && e.cell_b == neighbour;
          wall_between_grid |= e.cell_b == current && e.cell_a == neighbour;
        }

        if (wall_between_grid)
          continue; // impassable

        //
      }

      if (neighbour_cost == -1)
        continue; // impassable

      int new_cost = cost_so_far[current] + neighbour_cost;

      if (!cost_so_far.contains(neighbour) || new_cost < cost_so_far[neighbour]) {
        cost_so_far[neighbour] = new_cost;
        int priority = new_cost + heuristic<vec2i>(neighbour, to);
        frontier.enqueue(neighbour, priority);
        came_from[neighbour] = current;
      }
    }
  }

  return {};
}

std::vector<glm::ivec2>
generate_direct_with_diagonals(entt::registry& r, const GridComponent& grid, const int from_idx, const int to_idx)
{
  std::vector<astar_cell> map = generate_map_view(r, grid);

  std::vector<vec2i> path;

  const auto from_glm = engine::grid::index_to_grid_position(from_idx, grid.width, grid.height);
  const auto to_glm = engine::grid::index_to_grid_position(to_idx, grid.width, grid.height);
  const vec2i from{ from_glm.x, from_glm.y };
  const vec2i to{ to_glm.x, to_glm.y };

  if (equal<vec2i>(from, to))
    return {};

  PriorityQueue<vec2i> frontier;
  frontier.enqueue(from, 0);
  std::map<vec2i, vec2i> came_from;
  std::map<vec2i, int> cost_so_far;
  came_from[from] = from;
  cost_so_far[from] = 0;

  while (frontier.size() > 0) {
    const auto current = frontier.dequeue();

    if (equal<vec2i>(current, to))
      return reconstruct_path(came_from, from, to);

    const auto neighbours_idxs =
      engine::grid::get_neighbour_indicies_with_diagonals(current.x, current.y, grid.width, grid.height);

    for (const auto& [dir, idx] : neighbours_idxs) {
      const auto& neighbour = map[idx].pos;
      const auto& neighbour_cost = map[idx].cost;

      if (neighbour_cost == -1)
        continue; // impassable

      int new_cost = cost_so_far[current] + neighbour_cost;

      if (!cost_so_far.contains(neighbour) || new_cost < cost_so_far[neighbour]) {
        cost_so_far[neighbour] = new_cost;
        int priority = new_cost + heuristic<vec2i>(neighbour, to);
        frontier.enqueue(neighbour, priority);
        came_from[neighbour] = current;
      }
    }
  }

  return {};
};

[[nodiscard]] std::vector<glm::ivec2>
generate_accessible_areas(entt::registry& r, const GridComponent& grid, const int from_idx, const int range)
{
  std::vector<astar_cell> map = generate_map_view(r, grid);
  map[from_idx].distance = 0;

  const auto from_glm = engine::grid::index_to_grid_position(from_idx, grid.width, grid.height);
  const vec2i from{ from_glm.x, from_glm.y };

  PriorityQueue<vec2i> frontier;
  frontier.enqueue(from, 0);

  std::vector<glm::ivec2> results;

  while (frontier.size() > 0) {
    const auto current = frontier.dequeue();
    const auto current_idx = engine::grid::grid_position_to_index({ current.x, current.y }, grid.width);
    results.push_back({ current.x, current.y });

    // check neighbours
    const auto neighbours_idxs = engine::grid::get_neighbour_indicies(current.x, current.y, grid.width, grid.height);

    for (const auto& [dir, idx] : neighbours_idxs) {
      auto& neighbour = map[idx];

      if (neighbour.cost == -1)
        continue; // impassable

      int distance = map[current_idx].distance + 1;
      if (distance > range)
        continue;

      if (neighbour.distance == INT_MAX) {
        neighbour.distance = distance;
        frontier.enqueue(neighbour.pos, 0);
      } else if (distance < neighbour.distance) {
        neighbour.distance = distance;
      }
    }
  }

  return results;
};

std::vector<astar_cell>
generate_flow_field(entt::registry& r, const GridComponent& grid, const int from_idx)
{
  std::vector<astar_cell> map = generate_map_view(r, grid);

  const int from = grid.grid.size() - 1;
  const int to = 0;
  const vec2i to_pos = { 0, 0 };

  PriorityQueue<int> frontier;
  frontier.enqueue(from, 0);
  std::map<int, int> came_from;
  std::map<int, int> cost_so_far; // index, distance
  came_from[from] = from;
  cost_so_far[0] = 0;

  while (frontier.size() > 0) {
    auto current = frontier.dequeue();

    const auto gpos = engine::grid::index_to_grid_position(current, grid.width, grid.height);
    const auto neighbours_idxs = engine::grid::get_neighbour_indicies(gpos.x, gpos.y, grid.width, grid.height);

    for (const auto& [dir, neighbour_idx] : neighbours_idxs) {
      const auto& neighbour_pos = map[neighbour_idx].pos;
      const auto& neighbour_cost = map[neighbour_idx].cost;

      if (neighbour_cost == -1)
        continue; // impasable

      int new_cost = cost_so_far[current] + neighbour_cost;

      if (!cost_so_far.contains(neighbour_idx) || new_cost < cost_so_far[neighbour_idx]) {
        cost_so_far[neighbour_idx] = new_cost;
        int priority = new_cost + heuristic<vec2i>(neighbour_pos, to_pos);
        frontier.enqueue(neighbour_idx, priority);
        came_from[neighbour_idx] = current;
      }
    }
  } // end while loop

  // set final distances
  for (int xy = 0; xy < grid.width * grid.height; xy++) {
    if (map[xy].cost == -1)
      continue;
    if (auto it{ cost_so_far.find(xy) }; it != std::end(cost_so_far)) {
      const auto& [key, val]{ *it };
      map[xy].distance = val;
    } else
      map[xy].distance = INT_MAX;
  }
  map[0].distance = INT_MAX;

  return map;
};

// update visuals
void
display_flow_field_with_visuals(entt::registry& r, GridComponent& grid)
{
  std::vector<astar_cell> map = generate_map_view(r, grid);

  for (auto& vfx_ents : grid.debug_flow_field) {
    r.destroy(vfx_ents.begin(), vfx_ents.end());
    vfx_ents.clear();
  }
  grid.debug_flow_field.clear();
  grid.debug_flow_field.resize(grid.width * grid.height);

  for (int xy = 0; xy < grid.width * grid.height; xy++) {
    const auto gpos = engine::grid::index_to_grid_position(xy, grid.width, grid.height);

    const int text_seperation = 10;
    const int distance = map[xy].distance;
    if (distance == INT_MAX)
      continue;
    const auto sprites = convert_int_to_sprites(distance);
    auto& sprite_vfxs = grid.debug_flow_field[xy];
    for (int i = 0; i < sprites.size(); i++) {
      const auto sprite = create_gameplay(r, EntityType::empty_with_transform);
      set_sprite(r, sprite, sprites[i]);

      glm::ivec2 position = { gpos.x * grid.size, gpos.y * grid.size };
      position.x += (i + 1) * text_seperation;
      set_position(r, sprite, position);

      sprite_vfxs.push_back(sprite);
    }
  }
};

glm::ivec2
clamp_worldspace_to_gridspace(const MapComponent& map, const glm::ivec2 pos)
{
  auto gridpos = engine::grid::world_space_to_grid_space(pos, map.tilesize);
  gridpos.x = glm::clamp(gridpos.x, 0, map.xmax - 1);
  gridpos.y = glm::clamp(gridpos.y, 0, map.ymax - 1);
  return gridpos;
};

int
convert_position_to_index(const MapComponent& map, const glm::ivec2& pos)
{
  const auto gridpos = clamp_worldspace_to_gridspace(map, pos);
  return engine::grid::grid_position_to_index(gridpos, map.xmax);
};

bool
has_destination(entt::registry& r, const entt::entity& src_e)
{
  if (auto* existing_path = r.try_get<GeneratedPathComponent>(src_e))
    return existing_path->path.size() > 0;
  return false;
};

bool
at_destination(entt::registry& r, const entt::entity& src_e)
{
  const auto& map = get_first_component<MapComponent>(r);

  const auto src = get_position(r, src_e);
  const auto src_idx = convert_position_to_index(map, src);
  const auto src_gridpos = engine::grid::index_to_grid_position(src_idx, map.xmax, map.ymax);

  const auto& path = r.get<GeneratedPathComponent>(src_e);
  const auto last = path.path[path.path.size() - 1];

  const bool same_gridcell = last == src_gridpos;
  if (!same_gridcell)
    return false;

  // require distance threshold from position.
  const auto d = src - path.dst_pos;
  const int d2 = d.x * d.x + d.y * d.y;
  const int epsilon = 10;
  return d2 < epsilon;
};

bool
destination_is_blocked(entt::registry& r, const glm::ivec2 worldspace_pos)
{
  const auto& map = get_first_component<MapComponent>(r);
  const auto idx = convert_position_to_index(map, worldspace_pos);

  for (const auto& ent : map.map[idx]) {
    const auto& comp = r.get<PathfindComponent>(ent);
    if (comp.cost == -1)
      return true;
  }

  return false;
}

} // namespace game2d