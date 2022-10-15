#include "helpers.hpp"

#include "components.hpp"
#include "engine/maths/grid.hpp"
#include "modules/events/helpers/mouse.hpp"
#include "modules/physics/components.hpp"
#include "modules/renderer/components.hpp"

namespace game2d {

void
update_renderable(entt::registry& r, const entt::entity& e, const int x, const int y, const int width, const int height)
{
  auto& transform = r.get<TransformComponent>(e);
  transform.position.x = x;
  transform.position.y = y;
  transform.scale.x = width;
  transform.scale.y = height;
};

void
update_free_cursor(entt::registry& r, glm::ivec2 mouse_position)
{
  for (auto [entity, c] : r.view<FreeCursorComponent>().each()) {
    // update mouse_click
    if (get_mouse_lmb_press())
      c.mouse_click = mouse_position;

    // update mouse_width

    // regular square
    int width = c.CURSOR_SIZE;
    int height = c.CURSOR_SIZE;

    // expanding square
    if (get_mouse_lmb_press() || get_mouse_lmb_held() || get_mouse_lmb_release()) {
      width = mouse_position.x - c.mouse_click.x;
      height = mouse_position.y - c.mouse_click.y;
    }

    //
    // Rendering...
    //
    int x_offset = width;
    int x_offset_half = width / 2.0f;
    int y_offset = height;
    int y_offset_half = height / 2.0f;

    int tx = 0;
    int ty = 0;
    int bx = 0;
    int by = 0;
    int lx = 0;
    int ly = 0;
    int rx = 0;
    int ry = 0;
    if (get_mouse_lmb_press() || get_mouse_lmb_held() || get_mouse_lmb_release()) {
      // draw a cursor from the clicked point as the top left
      tx = c.mouse_click.x + x_offset_half;
      ty = c.mouse_click.y;
      bx = c.mouse_click.x + x_offset_half;
      by = c.mouse_click.y + y_offset;
      lx = c.mouse_click.x;
      ly = c.mouse_click.y + y_offset_half;
      rx = c.mouse_click.x + x_offset;
      ry = c.mouse_click.y + y_offset_half;
    } else {
      // draw a cursor around the cursor as the center point
      tx = mouse_position.x;
      ty = mouse_position.y - y_offset_half;
      bx = mouse_position.x;
      by = mouse_position.y + y_offset_half;
      lx = mouse_position.x - x_offset_half;
      ly = mouse_position.y;
      rx = mouse_position.x + x_offset_half;
      ry = mouse_position.y;
    }

    // set cursor renderables
    update_renderable(r, c.line_u, tx, ty, width, 1);
    update_renderable(r, c.line_d, bx, by, width, 1);
    update_renderable(r, c.line_l, lx, ly, 1, height);
    update_renderable(r, c.line_r, rx, ry, 1, height);
    update_renderable(r, c.backdrop, tx, ly, width, height);
  }
};

void
update_grid_cursor(entt::registry& r, glm::ivec2 mouse_position){

  // for (auto [entity, c] : r.view<GridCursorComponent>().each()) {
  //   glm::ivec2 mouse_position_clamped = engine::grid::world_space_to_clamped_world_space(mouse_position,
  //   c.GRID_SIZE);

  //   // update mouse_click
  //   if (get_mouse_lmb_press())
  //     c.mouse_click = mouse_position;

  //   // update mouse_width

  //   // regular square
  //   int width = c.CURSOR_SIZE;
  //   int height = c.CURSOR_SIZE;

  //   // expanding square
  //   if (get_mouse_lmb_press() || get_mouse_lmb_held() || get_mouse_lmb_release()) {
  //     width = mouse_position.x - c.mouse_click.x;
  //     height = mouse_position.y - c.mouse_click.y;
  //   }

  //   //
  //   // Rendering...
  //   //
  //   int x_offset = width;
  //   int x_offset_half = width / 2.0f;
  //   int y_offset = height;
  //   int y_offset_half = height / 2.0f;

  //   int tx = 0;
  //   int ty = 0;
  //   int bx = 0;
  //   int by = 0;
  //   int lx = 0;
  //   int ly = 0;
  //   int rx = 0;
  //   int ry = 0;
  //   if (get_mouse_lmb_press() || get_mouse_lmb_held() || get_mouse_lmb_release()) {
  //     // draw a cursor from the clicked point as the top left
  //     tx = c.mouse_click.x + x_offset_half;
  //     ty = c.mouse_click.y;
  //     bx = c.mouse_click.x + x_offset_half;
  //     by = c.mouse_click.y + y_offset;
  //     lx = c.mouse_click.x;
  //     ly = c.mouse_click.y + y_offset_half;
  //     rx = c.mouse_click.x + x_offset;
  //     ry = c.mouse_click.y + y_offset_half;
  //   } else {
  //     // draw a cursor around the cursor as the center point
  //     tx = mouse_position.x;
  //     ty = mouse_position.y - y_offset_half;
  //     bx = mouse_position.x;
  //     by = mouse_position.y + y_offset_half;
  //     lx = mouse_position.x - x_offset_half;
  //     ly = mouse_position.y;
  //     rx = mouse_position.x + x_offset_half;
  //     ry = mouse_position.y;
  //   }

  //   // set cursor renderables
  //   update_renderable(r, c.line_u, tx, ty, width, 1);
  //   update_renderable(r, c.line_d, bx, by, width, 1);
  //   update_renderable(r, c.line_l, lx, ly, 1, height);
  //   update_renderable(r, c.line_r, rx, ry, 1, height);
  //   update_renderable(r, c.backdrop, tx, ly, width, height);
  // }
};

} // namespace game2d