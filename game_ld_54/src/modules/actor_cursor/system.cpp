#include "system.hpp"

#include "components.hpp"
#include "entt/helpers.hpp"
#include "maths/grid.hpp"
#include "modules/grid/components.hpp"
#include "modules/renderer/components.hpp"
#include "physics/components.hpp"

namespace game2d {

void
update_cursor_system(entt::registry& r, const glm::ivec2& mouse_pos)
{
  static bool clamp_to_grid = true;

  // check grid exists
  const auto grid_e = get_first<GridComponent>(r);
  if (grid_e == entt::null)
    return;
  const auto& grid = r.get<GridComponent>(grid_e);

  const auto& view = r.view<TransformComponent, AABB, CursorComponent>();
  for (const auto& [entity, transform, aabb, cursor] : view.each()) {

    auto pos = mouse_pos;

    if (clamp_to_grid) {
      // so that the grid cursor doesnt swap around the center of the tile,
      // and that we swap when we move past the top left,
      // add an offset to the mouse position
      const glm::ivec2 offset = { grid.size / 2, grid.size / 2 };

      const auto grid_x_min = -offset.x;
      const auto grid_x_max = grid.width * grid.size - offset.x;
      const auto grid_y_min = -offset.y;
      const auto grid_y_max = grid.height * grid.size - offset.y;

      if (pos.x < grid_x_max && pos.x > grid_x_min && pos.y > grid_y_min && pos.y < grid_y_max) {
        const auto grid_xy = engine::grid::world_space_to_grid_space(pos + offset, grid.size);
        auto grid_idx = engine::grid::grid_position_to_index(grid_xy, grid.width);
        grid_idx = glm::clamp(grid_idx, 0, (grid.width * grid.height) - 1);
        pos = engine::grid::index_to_world_position(grid_idx, grid.width, grid.height, grid.size);
        cursor.grid_index = grid_idx;
      }
    }
    transform.position.x = pos.x;
    transform.position.y = pos.y;
    transform.scale.x = grid.size;
    transform.scale.y = grid.size;
    aabb.center.x = pos.x;
    aabb.center.y = pos.y;
    aabb.size = { grid.size, grid.size };
  }
}

} // namespace game2d