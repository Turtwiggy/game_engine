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
  const auto& grid = get_first_component<GridComponent>(r);

  const auto& view = r.view<TransformComponent, AABB, const CursorComponent>();
  for (const auto& [entity, transform, aabb, cursor] : view.each()) {

    auto pos = mouse_pos;
    if (clamp_to_grid) {
      // so that the grid cursor doesnt swap around the center of the tile,
      // and that we swap when we move past the top left,
      // add an offset to the mouse position
      const glm::ivec2 offset = { grid.size / 2, grid.size / 2 };

      pos = engine::grid::world_space_to_clamped_world_space(mouse_pos + offset, grid.size);
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