#include "helpers.hpp"

#include "actors/helpers.hpp"
#include "entt/helpers.hpp"
#include "maths/grid.hpp"
#include "modules/algorithm_astar_pathfinding/components.hpp"
#include "modules/algorithm_astar_pathfinding/priority_queue.hpp"
#include "modules/grid/components.hpp"


#include <fmt/core.h>
#include <map>

namespace game2d {

// Converts MapComponent to std::vector<astar_cell>
std::vector<astar_cell>
generate_map_view(entt::registry& r, const MapComponent& map)
{
  std::vector<astar_cell> result;

  for (int xy = 0; xy < map.xmax * map.ymax; xy++) {
    const auto gpos = engine::grid::index_to_grid_position(xy, map.xmax, map.ymax);

    astar_cell cell;
    cell.idx = xy;
    cell.pos = { gpos.x, gpos.y };

    // If the cell contains entites with cost info,
    // add that cost info to the cost
    // for (const auto& ent : map.map[xy]) {
    //   if (auto* cost = r.try_get<PathfindComponent>(ent))
    //     cell.cost = cost->cost;
    // }

    if (map.map[xy] != entt::null) {
      if (auto* pfc = r.try_get<PathfindComponent>(map.map[xy]))
        cell.cost = pfc->cost;
    }

    result.push_back(cell);
  }

  // fmt::println("map size: " << result.size() << std::endl;
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
      fmt::println("(error) pathfinding hit a loop.");
      break;
    }
  }

  path.push_back({ from.x, from.y }); // optional to add end
  std::reverse(path.begin(), path.end());
  return path;
}

std::vector<glm::ivec2>
generate_direct(entt::registry& r,
                const MapComponent& map_c,
                const int from_idx,
                const int to_idx,
                const std::optional<std::vector<Edge>> edges)
{
  // this seems bad
  const std::vector<astar_cell> map = generate_map_view(r, map_c);

  std::vector<vec2i> path;

  const auto from_glm = engine::grid::index_to_grid_position(from_idx, map_c.xmax, map_c.ymax);
  const auto to_glm = engine::grid::index_to_grid_position(to_idx, map_c.xmax, map_c.ymax);
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
    const vec2i current = frontier.dequeue();
    const int current_idx = map_c.xmax * current.y + current.x;

    if (equal<vec2i>(current, to))
      return reconstruct_path(came_from, from, to);

    const auto neighbour_idxs = engine::grid::get_neighbour_indicies(current.x, current.y, map_c.xmax, map_c.ymax);

    for (const auto& [dir, n_idx] : neighbour_idxs) {
      const auto& neighbour = map[n_idx].pos;
      const auto& neighbour_cost = map[n_idx].cost;

      // Check the edges
      if (edges.has_value()) {
        // Check if there's an edge between the current cell and the neighbour cell.
        // If there is, skip evaluating this neighbour.
        const std::vector<Edge>& es = edges.value();

        bool wall_between_grid = false;
        for (const Edge& e : es) {
          wall_between_grid |= e.a_idx == current_idx && e.b_idx == n_idx;
          wall_between_grid |= e.b_idx == current_idx && e.a_idx == n_idx;
        }
        if (wall_between_grid)
          continue; // impassable
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
generate_direct_with_diagonals(entt::registry& r, const MapComponent& map_c, const int from_idx, const int to_idx)
{
  const std::vector<astar_cell> map = generate_map_view(r, map_c);

  std::vector<vec2i> path;

  const auto from_glm = engine::grid::index_to_grid_position(from_idx, map_c.xmax, map_c.ymax);
  const auto to_glm = engine::grid::index_to_grid_position(to_idx, map_c.xmax, map_c.ymax);
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
      engine::grid::get_neighbour_indicies_with_diagonals(current.x, current.y, map_c.xmax, map_c.ymax);

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
generate_accessible_areas(entt::registry& r, const MapComponent& map_c, const int from_idx, const int range)
{
  std::vector<astar_cell> map = generate_map_view(r, map_c);
  map[from_idx].distance = 0;

  const auto from_glm = engine::grid::index_to_grid_position(from_idx, map_c.xmax, map_c.ymax);
  const vec2i from{ from_glm.x, from_glm.y };

  PriorityQueue<vec2i> frontier;
  frontier.enqueue(from, 0);

  std::vector<glm::ivec2> results;

  while (frontier.size() > 0) {
    const auto current = frontier.dequeue();
    const auto current_idx = engine::grid::grid_position_to_index({ current.x, current.y }, map_c.xmax);
    results.push_back({ current.x, current.y });

    // check neighbours
    const auto neighbours_idxs = engine::grid::get_neighbour_indicies(current.x, current.y, map_c.xmax, map_c.ymax);

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

/*
std::vector<astar_cell>
generate_flow_field(entt::registry& r, const MapComponent& map_c, const int from_idx)
{
  std::vector<astar_cell> map = generate_map_view(r, map_c);

  const int from = static_cast<int>(map_c.map.size() - 1);
  const int to = 0;
  const vec2i to_pos = { 0, 0 };

  PriorityQueue<int> frontier;
  frontier.enqueue(from, 0);
  std::map<int, int> came_from;
  std::map<int, int> cost_so_far; // index, distance
  came_from[from] = from;
  cost_so_far[0] = 0;

  while (frontier.size() > 0) {
    const int current = static_cast<int>(frontier.dequeue());

    const auto gpos = engine::grid::index_to_grid_position(current, map_c.xmax, map_c.ymax);
    const auto neighbours_idxs = engine::grid::get_neighbour_indicies(gpos.x, gpos.y, map_c.xmax, map_c.ymax);

    for (const auto& [dir, neighbour_idx] : neighbours_idxs) {
      const auto& neighbour_pos = map[neighbour_idx].pos;
      const auto& neighbour_cost = map[neighbour_idx].cost;

      if (neighbour_cost == -1)
        continue; // impasable

      const int new_cost = cost_so_far[current] + neighbour_cost;

      if (!cost_so_far.contains(neighbour_idx) || new_cost < cost_so_far[neighbour_idx]) {
        cost_so_far[neighbour_idx] = new_cost;
        const int priority = new_cost + heuristic<vec2i>(neighbour_pos, to_pos);
        frontier.enqueue(neighbour_idx, priority);
        came_from[neighbour_idx] = current;
      }
    }
  } // end while loop

  // set final distances
  for (int xy = 0; xy < map_c.xmax * map_c.ymax; xy++) {
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
*/

// update visuals
// void
// display_flow_field_with_visuals(entt::registry& r, MapComponent& map_c)
// {
//   std::vector<astar_cell> map = generate_map_view(r, grid);

//   for (auto& vfx_ents : grid.debug_flow_field) {
//     r.destroy(vfx_ents.begin(), vfx_ents.end());
//     vfx_ents.clear();
//   }
//   grid.debug_flow_field.clear();
//   grid.debug_flow_field.resize(map.xmax * map.ymax);

//   for (int xy = 0; xy < map.xmax * map.ymax; xy++) {
//     const auto gpos = engine::grid::index_to_grid_position(xy, map.xmax, map.ymax);

//     const int text_seperation = 10;
//     const int distance = map[xy].distance;
//     if (distance == INT_MAX)
//       continue;
//     const auto sprites = convert_int_to_sprites(distance);
//     auto& sprite_vfxs = grid.debug_flow_field[xy];
//     for (int i = 0; i < sprites.size(); i++) {
//       const auto sprite = create_gameplay(r, EntityType::empty_with_transform);
//       set_sprite(r, sprite, sprites[i]);

//       glm::ivec2 position = { gpos.x * grid.size, gpos.y * grid.size };
//       position.x += (i + 1) * text_seperation;
//       set_position(r, sprite, position);

//       sprite_vfxs.push_back(sprite);
//     }
//   }
// };

bool
has_destination(entt::registry& r, const entt::entity src_e)
{
  if (auto* existing_path = r.try_get<GeneratedPathComponent>(src_e))
    return existing_path->path.size() > 0;
  return false;
};

bool
at_destination(entt::registry& r, const entt::entity src_e)
{
  const auto& map = get_first_component<MapComponent>(r);

  const auto src = get_position(r, src_e);
  const auto src_idx = engine::grid::worldspace_to_index(src, map.tilesize, map.xmax, map.ymax);
  const auto src_gridpos = engine::grid::index_to_grid_position(src_idx, map.xmax, map.ymax);

  const auto& path = r.try_get<GeneratedPathComponent>(src_e);

  // no path means at path?
  if (path == nullptr)
    return true;

  const auto last = path->path[path->path.size() - 1];
  const bool same_gridcell = last == src_gridpos;
  if (!same_gridcell)
    return false;

  // require distance threshold from position.
  const glm::vec2 d = src - path->dst_pos;
  const float d2 = d.x * d.x + d.y * d.y;
  const float epsilon = 10;
  return d2 < epsilon;
};

bool
destination_is_blocked(entt::registry& r, const glm::ivec2 worldspace_pos)
{
  auto& map = get_first_component<MapComponent>(r);
  const auto idx = engine::grid::worldspace_to_index(worldspace_pos, map.tilesize, map.xmax, map.ymax);

  return map.map[idx] != entt::null;
  /*
  for (const auto& e : map.map[idx]) {
    if (!r.valid(e)) {
      fmt::println("something invalid in map.map...");
      continue;
    }

    if (e == you)
      continue; // you cant block yourself

    // something exists, so it should have a pathfind component
    const auto& comp = r.get<PathfindComponent>(e);
    if (comp.cost == -1)
      return true;

    // if there's anything at the destination, consider it blocked.
    return true;
  }
  */
  return false;
}

} // namespace game2d