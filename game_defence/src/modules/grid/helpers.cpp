#include "helpers.hpp"

#include "components.hpp"
#include "entt/helpers.hpp"
#include "maths/grid.hpp"
#include "modules/actors/helpers.hpp"
#include "modules/algorithm_astar_pathfinding/helpers.hpp"
#include "modules/system_turnbased_enemy/components.hpp"

#include <fmt/core.h>

namespace game2d {

GridComponent
map_to_grid(entt::registry& r)
{
  auto& map = get_first_component<MapComponent>(r);
  GridComponent grid; // Convert Map to Grid (?)
  grid.size = map.tilesize;
  grid.width = map.xmax;
  grid.height = map.ymax;
  grid.grid = map.map;
  return grid;
};

void
move_entity_on_map(entt::registry& r, const entt::entity& src_e, const glm::ivec2& dst)
{
  auto& map = get_first_component<MapComponent>(r);

  const auto src = get_position(r, src_e);
  const auto src_idx = convert_position_to_index(map, src);
  const auto dst_idx = convert_position_to_index(map, dst);

  // remove from current gridpos
  const auto hmm =
    std::remove_if(map.map[src_idx].begin(), map.map[src_idx].end(), [&src_e](const entt::entity& a) { return a == src_e; });
  map.map[src_idx].erase(hmm, map.map[src_idx].end());

  // immediately update the map so that other entities would prefer not to pathfind to the destination
  map.map[dst_idx].push_back(src_e);
};

int
get_lowest_cost_neighbour(entt::registry& r,
                          const MapComponent& map,
                          const GridComponent& grid,
                          const int src_idx,
                          const entt::entity& e)
{
  const auto dst = get_position(r, e);
  const auto dst_idx = convert_position_to_index(map, dst);
  const auto dst_gridpos = engine::grid::index_to_grid_position(dst_idx, map.xmax, map.ymax);

  // Warning: using diagonal neighbour indicies would require
  // working out if there's an edge between diagonals
  const auto neighbour_idxs = engine::grid::get_neighbour_indicies(dst_gridpos.x, dst_gridpos.y, grid.width, grid.height);

  // If you're already at one of the neighbour indexs, dont move!
  for (const auto& [dir, n_idx] : neighbour_idxs)
    if (n_idx == src_idx)
      return n_idx;

  // get cost at each neighbour
  std::vector<std::pair<int, int>> idx_to_cost;
  for (const auto& [dir, n_idx] : neighbour_idxs) {

    // take in to account edges
    const vec2i current = dst_gridpos;
    const vec2i neighbour = engine::grid::index_to_grid_position(n_idx, map.xmax, map.ymax);
    bool wall_between_grid = false;
    for (const Edge& e : map.edges) {
      wall_between_grid |= e.cell_a == current && e.cell_b == neighbour;
      wall_between_grid |= e.cell_b == current && e.cell_a == neighbour;
    }

    // dont get neighbours if there is an edge between
    if (wall_between_grid) {
      idx_to_cost.push_back({ n_idx, -1 });
      continue; // impassable
    }

    auto& map_entries = map.map[n_idx];
    for (const auto& map_e : map_entries) {
      const auto& pathfinding_c = r.get<PathfindComponent>(map_e);
      idx_to_cost.push_back({ n_idx, pathfinding_c.cost });
      continue;
    }

    // destination has no cost, because it has nothing there.
    if (map_entries.size() == 0)
      idx_to_cost.push_back({ n_idx, 0 });
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
update_path_to_tile_next_to_player(entt::registry& r, const entt::entity& src_e, const entt::entity& dst_e, const int limit)
{
  const auto& map = get_first_component<MapComponent>(r); // gets updated if units was dead
  const glm::ivec2 offset = { map.tilesize / 2, map.tilesize / 2 };
  const auto grid = map_to_grid(r);

  const auto src = get_position(r, src_e);
  const auto src_idx = convert_position_to_index(map, src);

  const auto chosen_neighbour_idx = get_lowest_cost_neighbour(r, map, grid, src_idx, dst_e);
  auto dst_pos = engine::grid::index_to_world_position(chosen_neighbour_idx, map.xmax, map.ymax, map.tilesize);
  dst_pos += offset;

  auto path = generate_direct(r, grid, src_idx, chosen_neighbour_idx, map.edges);

  // make sure path.size() < limit
  if (path.size() > limit) {
    // return +1, as usually the first path[0] is the element the entity is currently standing on
    const std::vector<glm::ivec2> path_limited(path.begin(), path.begin() + limit + 1);
    path = path_limited;
  }

  update_entity_path(r, src_e, path);
};

std::vector<glm::ivec2>
generate_path(entt::registry& r, const entt::entity& src_e, const glm::ivec2& worldspace_pos, const int limit)
{
  const auto& map = get_first_component<MapComponent>(r);
  const auto grid = map_to_grid(r);

  const auto src = get_position(r, src_e);
  const auto src_idx = convert_position_to_index(map, src);

  const auto worldspace_tl = engine::grid::world_space_to_clamped_world_space(worldspace_pos, map.tilesize);
  const auto worldspace_center = worldspace_tl + glm::ivec2{ map.tilesize / 2.0f, map.tilesize / 2.0f };
  const auto dst = worldspace_center;
  const int dst_idx = convert_position_to_index(map, dst);

  auto path = generate_direct(r, grid, src_idx, dst_idx, map.edges);

  // make sure path.size() < limit
  if (path.size() > limit) {
    // return +1, as usually the first path[0] is the element the entity is currently standing on
    const std::vector<glm::ivec2> path_limited(path.begin(), path.begin() + limit + 1);
    return path_limited;
  }

  return path;
};

void
update_entity_path(entt::registry& r, const entt::entity& src_e, const std::vector<glm::ivec2>& path)
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

  move_entity_on_map(r, src_e, dst);
};

} // namespace game2d