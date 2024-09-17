#include "helpers.hpp"

#include "actors/helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/maths/grid.hpp"
#include "modules/grid/components.hpp"
#include "modules/grid/helpers.hpp"
#include "modules/system_move_to_target_via_lerp/components.hpp"
#include "modules/system_turnbased/components.hpp"

namespace game2d {

void
move_action_common(entt::registry& r, const entt::entity e, const glm::vec2& dst_wp)
{
  const auto& map = get_first_component<MapComponent>(r);

  const auto info = get_entity_mapinfo(r, e);
  const auto src_idx = info->idx_in_map;

  // pathfinding...
  // note: use pathfinding here instead of direct movement
  // so that edges are taken in to account and you cant go through walls

  const auto dst_idx = engine::grid::worldspace_to_index(dst_wp, map.tilesize, map.xmax, map.ymax);
  const auto path = generate_path(r, src_idx, dst_idx, limit);

  /*
  fmt::print("path generated. ");
  for (const auto& p : path)
    fmt::print(" {},{}", p.x, p.y);
  fmt::println("");
  */

  if (path.size() < 2)
    return;
  const auto next_dst = path[1];
  const auto next_idx = engine::grid::grid_position_to_index(next_dst, map.xmax);

  // move action...
  const bool moved = move_entity_on_map(r, e, next_idx);
  if (!moved) {
    fmt::println("move_action_common(): entity didnt move...");
    return;
  }

  const int a = engine::grid::grid_position_to_index(get_grid_position(r, e), map.xmax);
  const int b = next_idx;

  // Lerp the player model, independent of the grid representation
  const auto offset = glm::vec2{ map.tilesize / 2.0f, map.tilesize / 2.0f };
  remove_if_exists<LerpToFixedTarget>(r, e);
  LerpToFixedTarget lerp;
  lerp.a = engine::grid::index_to_world_position(a, map.xmax, map.ymax, map.tilesize) + offset;
  lerp.b = engine::grid::index_to_world_position(b, map.xmax, map.ymax, map.tilesize) + offset;
  lerp.t = 0.0f;
  r.emplace<LerpToFixedTarget>(e, lerp);
};

} // namespace game2d