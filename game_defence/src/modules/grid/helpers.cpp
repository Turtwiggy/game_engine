#include "helpers.hpp"

#include "components.hpp"
#include "entt/helpers.hpp"
#include "maths/grid.hpp"
#include "modules/actors/helpers.hpp"
#include "modules/algorithm_astar_pathfinding/helpers.hpp"
#include "renderer/components.hpp"

#include <fmt/core.h>

#include <algorithm>

namespace game2d {

void
move_entity_on_map(entt::registry& r, const entt::entity src_e, const glm::vec2& dst)
{
  auto& map = get_first_component<MapComponent>(r);

  const auto src = get_position(r, src_e);
  const auto src_idx = convert_position_to_index(map, src);
  const auto dst_idx = convert_position_to_index(map, dst);

  map.map[src_idx] = entt::null;

  if (map.map[dst_idx] != entt::null) {
    auto& tag = r.get<TagComponent>(map.map[dst_idx]);
    fmt::println("warning: dst not clear for move. Tag: {}", tag.tag);
    return;
  }
  map.map[dst_idx] = src_e;

  // set_position(r, src_e, dst);
};

int
get_lowest_cost_neighbour(entt::registry& r, const MapComponent& map, const int src_idx, const entt::entity e)
{
  const auto dst = get_position(r, e);
  const auto dst_idx = convert_position_to_index(map, dst);
  const auto dst_gridpos = engine::grid::index_to_grid_position(dst_idx, map.xmax, map.ymax);

  // Warning: using diagonal neighbour indicies would require
  // working out if there's an edge between diagonals
  const auto neighbour_idxs = engine::grid::get_neighbour_indicies(dst_gridpos.x, dst_gridpos.y, map.xmax, map.ymax);

  // If you're already at one of the neighbour indexs, dont move!
  for (const auto& [dir, n_idx] : neighbour_idxs)
    if (n_idx == src_idx)
      return n_idx;

  // get cost at each neighbour
  std::vector<std::pair<int, int>> idx_to_cost;
  for (const auto& [dir, n_idx] : neighbour_idxs) {

    // take in to account edges
    bool wall_between_grid = false;
    for (const Edge& e : map.edges) {
      wall_between_grid |= e.a_idx == dst_idx && e.b_idx == n_idx;
      wall_between_grid |= e.b_idx == dst_idx && e.a_idx == n_idx;
    }

    // dont get neighbours if there is an edge between
    if (wall_between_grid) {
      idx_to_cost.push_back({ n_idx, -1 });
      continue; // impassable
    }
    /*
    auto& map_entries = map.map[n_idx];
    for (const auto& map_e : map_entries) {
      if (!r.valid(map_e))
        continue;
      const auto& pathfinding_c = r.get<PathfindComponent>(map_e);
      idx_to_cost.push_back({ n_idx, pathfinding_c.cost });
      continue;
    }

    // destination has no cost, because it has nothing there.
    if (map_entries.size() == 0)
      idx_to_cost.push_back({ n_idx, 0 });
    */

    const entt::entity entry = map.map[n_idx];
    if (entry == entt::null) // nothing there
      idx_to_cost.push_back({ n_idx, 0 });
    else {
      const auto& pathfinding_c = r.get<PathfindComponent>(entry);
      idx_to_cost.push_back({ n_idx, pathfinding_c.cost });
    }
  }

  // sort with lowest cost
  std::sort(idx_to_cost.begin(), idx_to_cost.end(), [](const auto& l, const auto& r) { return l.second < r.second; });

  // choose the first non -1 value
  for (const auto& [idx, cost] : idx_to_cost) {
    if (cost == -1)
      continue;
    return idx;
  }

  fmt::println("ERROR: all neighbours of desired cell have cost -1 or invalid");
  return idx_to_cost[0].first;
};

// Attach a GeneratedPath to the enemy unit. HasTargetPosition gets overwritten.
void
update_path_to_tile_next_to_player(entt::registry& r, const entt::entity src_e, const entt::entity dst_e, const int limit)
{
  const auto& map = get_first_component<MapComponent>(r); // gets updated if units was dead
  const glm::ivec2 offset = { map.tilesize / 2, map.tilesize / 2 };

  const auto src = get_position(r, src_e);
  const auto src_idx = convert_position_to_index(map, src);

  const auto chosen_neighbour_idx = get_lowest_cost_neighbour(r, map, src_idx, dst_e);
  auto dst_pos = engine::grid::index_to_world_position(chosen_neighbour_idx, map.xmax, map.ymax, map.tilesize);
  dst_pos += offset;

  auto path = generate_direct(r, map, src_idx, chosen_neighbour_idx, map.edges);

  // make sure path.size() < limit
  if (path.size() > static_cast<size_t>(limit)) {
    // return +1, as usually the first path[0] is the element the entity is currently standing on
    const std::vector<glm::ivec2> path_limited(path.begin(), path.begin() + limit + 1);
    path = path_limited;
  }

  update_entity_path(r, src_e, path);
};

std::vector<glm::ivec2>
generate_path(entt::registry& r, const entt::entity src_e, const glm::ivec2& worldspace_pos, const int limit)
{
  const auto& map = get_first_component<MapComponent>(r);

  const auto src = get_position(r, src_e);
  const auto src_idx = convert_position_to_index(map, src);

  const auto worldspace_tl = engine::grid::worldspace_to_clamped_world_space(worldspace_pos, map.tilesize);
  const auto worldspace_center = worldspace_tl + glm::ivec2{ map.tilesize / 2.0f, map.tilesize / 2.0f };
  const auto dst = worldspace_center;
  const int dst_idx = convert_position_to_index(map, dst);

  auto path = generate_direct(r, map, src_idx, dst_idx, map.edges);

  // make sure path.size() < limit
  if (path.size() > static_cast<size_t>(limit)) {
    // return +1, as usually the first path[0] is the element the entity is currently standing on
    const std::vector<glm::ivec2> path_limited(path.begin(), path.begin() + limit + 1);
    return path_limited;
  }

  return path;
};

void
update_entity_path(entt::registry& r, const entt::entity src_e, const std::vector<glm::ivec2>& path)
{
  if (path.size() == 0)
    return;
  const auto& map = get_first_component<MapComponent>(r);

  const glm::ivec2 worldspace_tl = engine::grid::grid_space_to_world_space(path[path.size() - 1], map.tilesize);
  const glm::ivec2 worldspace_center = worldspace_tl + glm::ivec2{ map.tilesize / 2.0f, map.tilesize / 2.0f };
  const glm::ivec2 dst = worldspace_center;

  GeneratedPathComponent path_c;
  path_c.path = path;
  path_c.src_pos = get_position(r, src_e);
  path_c.dst_pos = dst;
  path_c.dst_ent = entt::null;
  path_c.required_to_clear_path = true;
  path_c.wait_at_destination = true;
  path_c.path_cleared.resize(path.size());
  r.emplace_or_replace<GeneratedPathComponent>(src_e, path_c);

  // immediately move from source to end of path?
  move_entity_on_map(r, src_e, dst);
};

} // namespace game2d