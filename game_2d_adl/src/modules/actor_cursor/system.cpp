#include "system.hpp"

#include "components.hpp"
#include "entt/helpers.hpp"
#include "events/helpers/mouse.hpp"
#include "lifecycle/components.hpp"
#include "maths/grid.hpp"
#include "modules/renderer/components.hpp"
#include "physics/components.hpp"
#include "sprites/helpers.hpp"

namespace game2d {

static bool clamp_to_grid = false;

void
update_cursor_system(entt::registry& r, const glm::ivec2& mouse_pos)
{
  // check grid exists
  // const auto grid_e = get_first<GridComponent>(r);
  // if (grid_e == entt::null)
  //   return;
  // const auto& grid = r.get<GridComponent>(grid_e);

  // warning: doesnt work with controller currently
  const bool click = get_mouse_lmb_press();
  const bool held = get_mouse_lmb_held();
  const bool release = get_mouse_lmb_release();

  // What are we hovering?
  //

  const auto& first_cursor = get_first<CursorComponent>(r);

  static glm::ivec2 click_location = { 0, 0 };
  if (click) {
    click_location = mouse_pos;
    // auto& t = r.emplace<TransformComponent>(first_cursor);
    // t.scale = { 16, 16, 1 };
  }
  if (release) {
    // r.remove<TransformComponent>(first_cursor);
  }

  const auto& view = r.view<TransformComponent, AABB, CursorComponent>(entt::exclude<WaitForInitComponent>);
  for (const auto& [entity, transform, aabb, cursor] : view.each()) {

    // Is the cursor hovering any enemies?
    // auto& enemies = cursor.hovering_enemies;
    // enemies.clear();
    // const auto& hovering_view = r.view<const HoveredComponent, const EnemyComponent>(entt::exclude<WaitForInitComponent>);

    // // Update cursor if hovering enemies
    // //
    // if (hovering_view.size_hint() > 0 && !held)
    //   set_sprite(r, entity, "CURSOR_1");
    // else
    //   set_sprite(r, entity, "EMPTY");

    // if (clamp_to_grid) {
    //   // so that the grid cursor doesnt swap around the center of the tile,
    //   // and that we swap when we move past the top left,
    //   // add an offset to the mouse position
    //   const glm::ivec2 offset = { grid.size / 2, grid.size / 2 };

    //   const auto grid_x_min = -offset.x;
    //   const auto grid_x_max = grid.width * grid.size - offset.x;
    //   const auto grid_y_min = -offset.y;
    //   const auto grid_y_max = grid.height * grid.size - offset.y;

    //   if (pos.x < grid_x_max && pos.x > grid_x_min && pos.y > grid_y_min && pos.y < grid_y_max) {
    //     const auto grid_xy = engine::grid::world_space_to_grid_space(pos + offset, grid.size);
    //     auto grid_idx = engine::grid::grid_position_to_index(grid_xy, grid.width);
    //     grid_idx = glm::clamp(grid_idx, 0, (grid.width * grid.height) - 1);
    //     pos = engine::grid::index_to_world_position(grid_idx, grid.width, grid.height, grid.size);
    //     cursor.grid_index = grid_idx;
    //   }
    // } else
    //   cursor.grid_index = std::nullopt;

    // defaults
    glm::ivec2 size{ 32, 32 };
    glm::ivec2 pos = mouse_pos;

    // expand cursor when cursor is held
    if (held) {
      pos = (click_location + mouse_pos) / 2;
      size = glm::abs(mouse_pos - click_location);
    }

    transform.position.x = pos.x;
    transform.position.y = pos.y;
    aabb.center.x = pos.x;
    aabb.center.y = pos.y;
    transform.scale.x = size.x;
    transform.scale.y = size.y;
    aabb.size = { size.x, size.y };
  }
}

} // namespace game2d