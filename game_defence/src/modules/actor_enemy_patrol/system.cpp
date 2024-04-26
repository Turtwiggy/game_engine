#include "system.hpp"

#include "components.hpp"
#include "entt/helpers.hpp"
#include "lifecycle/components.hpp"
#include "maths/grid.hpp"
#include "maths/maths.hpp"
#include "modules/ai_pathfinding/helpers.hpp"
#include "modules/grid/components.hpp"
#include "modules/selected_interactions/components.hpp"
#include "physics/components.hpp"

#include <iostream>

namespace game2d {

// choose a new random position
// move towards position
// this behaviour is interrupted if a player gets within range

void
update_actor_enemy_patrol_system(entt::registry& r, const glm::ivec2 mouse_pos, const float dt)
{
  const auto& map_e = get_first<MapComponent>(r);
  if (map_e == entt::null)
    return;
  const auto& map = get_first_component<MapComponent>(r);

  // Convert Map to Grid (?)
  GridComponent grid;
  grid.size = map.tilesize;
  grid.width = map.xmax;
  grid.height = map.ymax;
  grid.grid = map.map;

  const auto convert_position_to_index = [&map](const glm::ivec2& src) -> int {
    auto src_gridpos = engine::grid::world_space_to_grid_space(src, map.tilesize);
    src_gridpos.x = glm::clamp(src_gridpos.x, 0, map.xmax - 1);
    src_gridpos.y = glm::clamp(src_gridpos.y, 0, map.ymax - 1);
    return engine::grid::grid_position_to_index(src_gridpos, map.xmax);
  };

  const auto& view = r.view<PatrolComponent, AABB>(entt::exclude<WaitForInitComponent>);
  for (const auto& [e, patrol_c, aabb] : view.each()) {
    //
    const auto src = aabb.center;
    const int src_idx = convert_position_to_index(src);

    const auto update_pathfinding = [&r, &e, &grid, &src_idx, &src, &map]() {
      std::cout << "updating pathfinding" << std::endl;

      // TODO: BAD. FIX.
      static engine::RandomState rnd;
      // should be any valid tiles, but for the moment, just choose a random one
      const int grid_max = (grid.width * grid.height) - 1;
      const int dst_idx = int(engine::rand_det_s(rnd.rng, 0, grid_max));
      const auto dst = engine::grid::index_to_world_position(dst_idx, map.xmax, map.ymax, map.tilesize);

      const auto path = generate_direct(r, grid, src_idx, dst_idx);
      GeneratedPathComponent path_c;
      path_c.path = path;
      path_c.src_pos = src;
      path_c.dst_pos = dst;
      path_c.path_cleared.resize(path.size());
      r.emplace_or_replace<GeneratedPathComponent>(e, path_c);
    };

    // Attach a GeneratedPath to the enemy unit. HasTargetPosition gets overwritten.
    const auto& existing_path = r.try_get<GeneratedPathComponent>(e);
    if (existing_path == NULL)
      update_pathfinding();
    else {
      // When to update path? If reached destination?
      const auto current_dst_idx = convert_position_to_index(existing_path->dst_pos);
      if (src_idx == current_dst_idx)
        update_pathfinding();
    }

    // Calculate distance to end position
    // const auto tgt_as_vec2 = glm::vec2(target_position.position.x, target_position.position.y);
    // const auto dir_raw = tgt_as_vec2 - glm::vec2(aabb.center);
    // const auto dir_nrm = engine::normalize_safe({ dir_raw.x, dir_raw.y });
    // const int d2 = dir_raw.x * dir_raw.x + dir_raw.y * dir_raw.y;
  }
}

} // namespace game2d