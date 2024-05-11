#include "system.hpp"

#include "entt/helpers.hpp"
#include "helpers/entity_pool.hpp"
#include "helpers/line.hpp"
#include "maths/grid.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/ai_pathfinding/helpers.hpp"
#include "modules/selected_interactions/components.hpp"
#include "modules/ux_hoverable/components.hpp"
#include "physics/components.hpp"

namespace game2d {

void
update_debug_pathfinding_system(entt::registry& r, const glm::ivec2& mouse_pos)
{
  const auto& map_e = get_first<MapComponent>(r);
  if (map_e == entt::null)
    return;
  const auto& map = get_first_component<MapComponent>(r);

  const auto convert_position_to_index = [&map](const glm::ivec2& src) -> int {
    auto src_gridpos = engine::grid::world_space_to_grid_space(src, map.tilesize);
    src_gridpos.x = glm::clamp(src_gridpos.x, 0, map.xmax - 1);
    src_gridpos.y = glm::clamp(src_gridpos.y, 0, map.ymax - 1);
    return engine::grid::grid_position_to_index(src_gridpos, map.xmax);
  };

  // Convert Map to Grid (?)
  GridComponent grid;
  grid.size = map.tilesize;
  grid.width = map.xmax;
  grid.height = map.ymax;
  grid.grid = map.map;

  const auto first_selected_e = get_first<SelectedComponent>(r);
  if (first_selected_e == entt::null)
    return;
  const auto& selected_aabb = r.get<AABB>(first_selected_e);

  const auto src = selected_aabb.center;
  const auto src_idx = convert_position_to_index(src);

  const auto dst = mouse_pos;
  const auto dst_idx = convert_position_to_index(dst);

  // display the path from the player to the mouse_pos
  const auto path = generate_direct(r, grid, src_idx, dst_idx);

  static EntityPool lines;
  lines.update(r, path.size() - 1);
  for (int i = 1; i < path.size(); i++) {
    const auto offset = glm::ivec2{ map.tilesize / 2, map.tilesize / 2 };
    glm::ivec2 cur = (glm::ivec2{ path[i].x, path[i].y } * map.tilesize) + offset;
    glm::ivec2 prv = (glm::ivec2{ path[i - 1].x, path[i - 1].y } * map.tilesize) + offset;
    set_transform_with_line(r, lines.instances[i - 1], generate_line(cur, prv, 2));
  }
};

} // namespace game2d