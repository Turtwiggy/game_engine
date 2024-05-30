#include "helpers.hpp"

#include "components.hpp"
#include "entt/helpers.hpp"
#include "maths/grid.hpp"
#include "modules/actors/helpers.hpp"
#include "modules/algorithm_astar_pathfinding/helpers.hpp"

#include <iostream>

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
get_lowest_cost_neighbour(entt::registry& r, const MapComponent& map, const GridComponent& grid, const entt::entity& e)
{
  const auto dst = get_position(r, e);
  const auto dst_idx = convert_position_to_index(map, dst);
  const auto dst_gridpos = engine::grid::index_to_grid_position(dst_idx, map.xmax, map.ymax);

  std::vector<std::pair<engine::grid::GridDirection, int>> neighbours_idxs;
  engine::grid::get_neighbour_indicies_with_diagonals(
    dst_gridpos.x, dst_gridpos.y, grid.width, grid.height, neighbours_idxs);

  // get cost at each neighbour
  std::vector<std::pair<int, int>> idx_to_cost;
  for (const auto& [dir, neighbour_idx] : neighbours_idxs) {
    const auto& map_entries = map.map[neighbour_idx];

    for (const auto& map_e : map_entries) {
      const auto& pathfinding_c = r.get<PathfindComponent>(map_e);
      idx_to_cost.push_back({ neighbour_idx, pathfinding_c.cost });
    }

    // destination has no cost, because it has nothing there.
    if (map_entries.size() == 0)
      idx_to_cost.push_back({ neighbour_idx, 0 });
  }

  // sort with lowest cost
  std::sort(idx_to_cost.begin(), idx_to_cost.end(), [](const auto& l, const auto& r) { return l.second < r.second; });

  // choose the first non -1 value
  for (const auto& [idx, cost] : idx_to_cost) {
    if (cost == -1)
      continue;
    return idx;
  }

  std::cerr << "ERROR: all neighbours of desired cell have cost -1 or invalid" << std::endl;
  return idx_to_cost[0].first;
};

// Attach a GeneratedPath to the enemy unit. HasTargetPosition gets overwritten.
void
update_path_to_tile_next_to_player(entt::registry& r, const entt::entity& src_e, const entt::entity& dst_e)
{
  const auto& map = get_first_component<MapComponent>(r);
  const glm::ivec2 offset = { map.tilesize / 2, map.tilesize / 2 };
  const auto grid = map_to_grid(r);

  const auto src = get_position(r, src_e);
  const auto src_idx = convert_position_to_index(map, src);

  const auto chosen_neighbour_idx = get_lowest_cost_neighbour(r, map, grid, dst_e);

  auto dst_pos = engine::grid::index_to_world_position(chosen_neighbour_idx, map.xmax, map.ymax, map.tilesize);
  dst_pos += offset;

  const auto path = generate_direct(r, grid, src_idx, chosen_neighbour_idx);
  GeneratedPathComponent path_c;
  path_c.path = path;
  path_c.src_pos = src;
  path_c.dst_pos = dst_pos;
  path_c.dst_ent = entt::null;
  path_c.aim_for_exact_position = true;
  path_c.required_to_clear_path = true;
  path_c.path_cleared.resize(path.size());
  r.emplace_or_replace<GeneratedPathComponent>(src_e, path_c);

  move_entity_on_map(r, src_e, dst_pos);
};

void
update_path_to_mouse(entt::registry& r, const entt::entity& src_e, const glm::ivec2& mouse_pos)
{
  const auto& map = get_first_component<MapComponent>(r);
  const auto grid = map_to_grid(r);

  const auto src = get_position(r, src_e);
  const auto src_idx = convert_position_to_index(map, src);

  const auto dst = mouse_pos;
  const int dst_idx = convert_position_to_index(map, dst);

  const auto path = generate_direct(r, grid, src_idx, dst_idx);
  GeneratedPathComponent path_c;
  path_c.path = path;
  path_c.src_pos = src;
  path_c.dst_pos = dst;
  path_c.dst_ent = entt::null;
  path_c.aim_for_exact_position = true;
  path_c.required_to_clear_path = true;
  path_c.path_cleared.resize(path.size());
  r.emplace_or_replace<GeneratedPathComponent>(src_e, path_c);

  move_entity_on_map(r, src_e, mouse_pos);
};

} // namespace game2d