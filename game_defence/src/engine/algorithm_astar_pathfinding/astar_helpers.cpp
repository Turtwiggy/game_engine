
#include "astar_helpers.hpp"

#include "actors/actor_helpers.hpp"
#include "astar_components.hpp"
#include "engine/algorithm_astar_pathfinding/astar_components.hpp"
#include "engine/algorithm_astar_pathfinding/priority_queue.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/maths/grid.hpp"
#include "modules/map/components.hpp"
#include "modules/spaceship_designer/generation/rooms_random.hpp"

#include <fmt/core.h>
#include <map>

namespace game2d {

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
};

int
get_cost_at_gridpos(entt::registry& r, const glm::ivec2 gp, const MapComponent& map_c)
{
  if (gp_out_of_bounds(gp, map_c.xmax, map_c.ymax))
    return 1;

  const auto idx = engine::grid::grid_position_to_index(gp, map_c.xmax);
  const auto& es = map_c.map[idx];

  int cost = 0;
  for (const auto e : es)
    if (const auto* p_c = r.try_get<PathfindComponent>(e))
      cost += p_c->cost;
  return cost;
};

bool
gridpos_blocked_by_map(entt::registry& r, const glm::ivec2& gp, const MapComponent& map_c)
{
  if (gp_out_of_bounds(gp, map_c.xmax, map_c.ymax))
    return false;

  return get_cost_at_gridpos(r, gp, map_c) == -1;
};

std::vector<glm::ivec2>
generate_direct(entt::registry& r, const vec2i from, const vec2i to)
{
  if (equal<vec2i>(from, to))
    return {};

  const auto& map_c = get_first_component<MapComponent>(r);

  std::vector<vec2i> path;

  PriorityQueue<vec2i> frontier;
  frontier.enqueue(from, 0);
  std::map<vec2i, vec2i> came_from;
  std::map<vec2i, int> cost_so_far;
  came_from[from] = from;
  cost_so_far[from] = 0;

  while (frontier.size() > 0) {

    // if ((int)frontier.size() > (map_c.xmax * map_c.ymax)) {
    //   fmt::println("highly likly pathfinding broke... ");
    //   break;
    // }

    const vec2i current = frontier.dequeue();
    const int current_idx = map_c.xmax * current.y + current.x;

    if (equal<vec2i>(current, to))
      return reconstruct_path(came_from, from, to);

    const auto neighbour_gps = engine::grid::get_neighbour_gridpos({ current.x, current.y }, map_c.xmax, map_c.ymax);

    for (const auto& [dir, gp] : neighbour_gps) {

      if (gp_out_of_bounds(gp, map_c.xmax, map_c.ymax))
        continue; // out of map

      if (gridpos_blocked_by_map(r, gp, map_c))
        continue; // impassable

      if (edge_between_gps(r, { current.x, current.y }, { gp.x, gp.y }) != entt::null)
        continue; // impassable

      // auto cur_in_room = inside_room(r, { current.x, current.y }).size() > 0;
      // auto nxt_in_room = inside_room(r, { gp.x, gp.y }).size() > 0;
      // if (cur_in_room && !nxt_in_room)
      //   continue; // dont search outside the ship

      const auto neighbour = gp;
      int neighbour_cost = get_cost_at_gridpos(r, gp, map_c);
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
generate_accessible_areas(entt::registry& r, const MapComponent& map_c, const vec2i from_pos, const int range)
{
  std::map<vec2i, int> pos_to_distance;
  pos_to_distance[from_pos] = 0;

  PriorityQueue<vec2i> frontier;
  frontier.enqueue(from_pos, 0);

  std::vector<glm::ivec2> results;

  while (frontier.size() > 0) {
    const auto current = frontier.dequeue();
    results.push_back({ current.x, current.y });

    // check neighbours
    const auto neighbour_gps = engine::grid::get_neighbour_gridpos({ current.x, current.y }, map_c.xmax, map_c.ymax);
    for (const auto& [dir, gp] : neighbour_gps) {

      if (gridpos_blocked_by_map(r, gp, map_c))
        continue; // impassable

      if (edge_between_gps(r, { current.x, current.y }, { gp.x, gp.y }) != entt::null)
        continue; // impassable

      int distance = pos_to_distance[current] + 1;
      if (distance > range)
        continue;

      // if a distance value already existed, take the smaller distance
      if (pos_to_distance.contains(gp))
        pos_to_distance[gp] = glm::min(distance, pos_to_distance[gp]);

      // no distance value: insert a new one
      else {
        pos_to_distance[gp] = distance;
        frontier.enqueue(gp, 0);
      }
    }

    //
  }

  return results;
};

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

  bool blocked = false;
  for (const auto& es : map.map) {
    for (const auto e : es) {
      if (const auto* pathfind_c = r.try_get<PathfindComponent>(e))
        if (pathfind_c->cost == -1)
          blocked |= true;
    }
  }

  return blocked;
};

entt::entity
edge_between_gps(entt::registry& r, const glm::ivec2& a, const glm::ivec2& b)
{
  const auto& view = r.view<const Edge>();

  const Edge tgt_edge(a, b);

  const auto it = std::find_if(view.begin(), view.end(), [&](entt::entity e) {
    const auto& edge = view.get<Edge>(e);
    return edge == tgt_edge;
  });

  if (it != view.end())
    return (*it);
  return entt::null;
};

bool
gp_out_of_bounds(const glm::ivec2& gp, const int xmax, const int ymax)
{
  if (gp.x < 0 || gp.x > xmax - 1)
    return true;
  if (gp.y < 0 || gp.y > ymax - 1)
    return true;

  return false;
}

} // namespace game2d

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

/*
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
*/