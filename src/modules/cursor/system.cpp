// your header
#include "system.hpp"

// components
#include "components.hpp"
#include "modules/events/components.hpp"
#include "modules/events/helpers/mouse.hpp"
#include "modules/physics/components.hpp"
#include "modules/renderer/components.hpp"

// other engine headers
#include "engine/maths/grid.hpp"
#include "engine/maths/maths.hpp"

// other lib headers
#include <glm/glm.hpp>

namespace game2d {
void
update_renderable(entt::registry& r, const entt::entity& e, const int x, const int y, const int w, const int h)
{
  auto& transform = r.get<TransformComponent>(e);
  transform.position.x = x;
  transform.position.y = y;
  transform.scale.x = w;
  transform.scale.y = h;
};

} // namespace game2d

void
game2d::update_cursor_system(entt::registry& registry)
{
  const auto& input = registry.ctx().at<SINGLETON_InputComponent>();

  int width = 0, height = 0;
  if (get_mouse_lmb_press() || get_mouse_lmb_held()) {
    // draw expanding square
    width = input.mouse_position_in_worldspace.x - input.mouse_click.x;
    height = input.mouse_position_in_worldspace.y - input.mouse_click.y;
  } else {
    // draw regular square
    width = input.CURSOR_SIZE;
    height = input.CURSOR_SIZE;
  }

  {
    const auto& view = registry.view<FreeCursorComponent>();
    view.each([&registry, &input, &width, &height](auto entity, auto& c) {
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
        tx = input.mouse_click.x + x_offset_half;
        ty = input.mouse_click.y;
        bx = input.mouse_click.x + x_offset_half;
        by = input.mouse_click.y + y_offset;
        lx = input.mouse_click.x;
        ly = input.mouse_click.y + y_offset_half;
        rx = input.mouse_click.x + x_offset;
        ry = input.mouse_click.y + y_offset_half;
      } else {
        // draw a cursor around the cursor as the center point
        tx = input.mouse_position_in_worldspace.x;
        ty = input.mouse_position_in_worldspace.y - y_offset_half;
        bx = input.mouse_position_in_worldspace.x;
        by = input.mouse_position_in_worldspace.y + y_offset_half;
        lx = input.mouse_position_in_worldspace.x - x_offset_half;
        ly = input.mouse_position_in_worldspace.y;
        rx = input.mouse_position_in_worldspace.x + x_offset_half;
        ry = input.mouse_position_in_worldspace.y;
      }

      // set cursor renderables
      update_renderable(registry, c.line_u, tx, ty, width, 1);
      update_renderable(registry, c.line_d, bx, by, width, 1);
      update_renderable(registry, c.line_l, lx, ly, 1, height);
      update_renderable(registry, c.line_r, rx, ry, 1, height);
      update_renderable(registry, c.backdrop, tx, ly, width, height);

      // set cursor physics
      auto& ps = registry.get<PhysicsSizeComponent>(entity);
      ps.w = width;
      ps.h = height;
    });
  }

  {
    const auto& view = registry.view<GridCursorComponent>();
    view.each([&registry, &input, &width, &height](auto entity, auto& c) {
      //
      // Rendering...
      //
      int x_offset = width;
      int x_offset_half = width / 2.0f;
      int y_offset = height;
      int y_offset_half = height / 2.0f;

      const int grid_size = 16;
      glm::ivec2 nearest_grid_tl =
        engine::grid::world_space_to_clamped_world_space(input.mouse_position_in_worldspace, grid_size);

      int tx = 0;
      int ty = 0;
      int bx = 0;
      int by = 0;
      int lx = 0;
      int ly = 0;
      int rx = 0;
      int ry = 0;
      if (get_mouse_lmb_press() || get_mouse_lmb_held() || get_mouse_lmb_release()) {

        glm::ivec2 click_grid_tl = engine::grid::world_space_to_clamped_world_space(input.mouse_click, grid_size);

        // draw a cursor from the clicked point as the top left
        tx = click_grid_tl.x + x_offset_half;
        ty = click_grid_tl.y;
        bx = click_grid_tl.x + x_offset_half;
        by = click_grid_tl.y + y_offset;
        lx = click_grid_tl.x;
        ly = click_grid_tl.y + y_offset_half;
        rx = click_grid_tl.x + x_offset;
        ry = click_grid_tl.y + y_offset_half;
      } else {
        // draw a cursor around the cursor as the center point
        tx = nearest_grid_tl.x;
        ty = nearest_grid_tl.y - y_offset_half;
        bx = nearest_grid_tl.x;
        by = nearest_grid_tl.y + y_offset_half;
        lx = nearest_grid_tl.x - x_offset_half;
        ly = nearest_grid_tl.y;
        rx = nearest_grid_tl.x + x_offset_half;
        ry = nearest_grid_tl.y;
      }

      // set cursor renderables
      update_renderable(registry, c.line_u, tx, ty, width, 1);
      update_renderable(registry, c.line_d, bx, by, width, 1);
      update_renderable(registry, c.line_l, lx, ly, 1, height);
      update_renderable(registry, c.line_r, rx, ry, 1, height);
      update_renderable(registry, c.backdrop, tx, ly, width, height);

      // set cursor physics
      auto& ps = registry.get<PhysicsSizeComponent>(entity);
      ps.w = width;
      ps.h = height;
    });
  }
};
