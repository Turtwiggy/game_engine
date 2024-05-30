#include "helpers.hpp"

#include "components.hpp"
#include "entt/helpers.hpp"
#include "maths/grid.hpp"
#include "modules/actors/helpers.hpp"
#include "modules/algorithm_astar_pathfinding/helpers.hpp"

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

// Attach a GeneratedPath to the enemy unit. HasTargetPosition gets overwritten.
void
update_path_to_tile_next_to_player(entt::registry& r, const entt::entity& src_e, const entt::entity& dst_e)
{
  const auto& map = get_first_component<MapComponent>(r);
  const glm::ivec2 offset = { map.tilesize / 2, map.tilesize / 2 };
  const auto grid = map_to_grid(r);

  const auto src = get_position(r, src_e);
  const auto src_idx = convert_position_to_index(map, src);

  const auto dst = get_position(r, dst_e);
  const auto dst_idx = convert_position_to_index(map, dst);
  const auto dst_gridpos = engine::grid::index_to_grid_position(dst_idx, map.xmax, map.ymax);

  std::vector<std::pair<engine::grid::GridDirection, int>> neighbours_idxs;
  engine::grid::get_neighbour_indicies(dst_gridpos.x, dst_gridpos.y, grid.width, grid.height, neighbours_idxs);

  // for the moment, just choose any neighbour index.
  // the better thing to do would be to check if it's full.

  for (const auto& [dir, neighbour_idx] : neighbours_idxs) {
    auto dst_pos = engine::grid::index_to_world_position(neighbour_idx, map.xmax, map.ymax, map.tilesize);
    dst_pos += offset;

    const auto path = generate_direct(r, grid, src_idx, neighbour_idx);
    GeneratedPathComponent path_c;
    path_c.path = path;
    path_c.src_pos = src;
    path_c.dst_pos = dst_pos;
    path_c.dst_ent = entt::null;
    path_c.aim_for_exact_position = true;
    path_c.required_to_clear_path = true;
    path_c.path_cleared.resize(path.size());
    r.emplace_or_replace<GeneratedPathComponent>(src_e, path_c);

    break; // first returned
  }
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
}

} // namespace game2d