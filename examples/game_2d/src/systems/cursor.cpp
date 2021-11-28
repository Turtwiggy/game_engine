// your header
#include "systems/cursor.hpp"

// components
#include "components/cursor.hpp"
#include "components/singleton_grid.hpp"
#include "modules/physics/components.hpp"
#include "modules/renderer/components.hpp"

// other engine headers
#include "engine/grid.hpp"
#include "engine/maths.hpp"

// other lib headers
#include <glm/glm.hpp>
#include <imgui.h>

void
game2d::update_cursor_system(entt::registry& registry, engine::Application& app)
{
  const auto& ri = registry.ctx<SINGLETON_RendererInfo>();
  const int& GRID_SIZE = registry.ctx<SINGLETON_GridSize>().size_xy;

  // dont process game events if the viewport says so
  if (!ri.viewport_process_events)
    return;

  glm::ivec2 imgui_mouse_pos = app.get_input().get_mouse_pos();
  glm::vec2 imgui_viewport_tl = ri.viewport_pos;
  glm::ivec2 mouse_pos = imgui_mouse_pos - glm::ivec2(imgui_viewport_tl.x, imgui_viewport_tl.y);
  glm::vec2 mouse_pos_adjusted_in_worldspace = mouse_pos;
  mouse_pos_adjusted_in_worldspace.x += GRID_SIZE / 2.0f;
  mouse_pos_adjusted_in_worldspace.y += GRID_SIZE / 2.0f;

  // Update cursor
  {
    const auto& view = registry.view<CursorComponent, PositionIntComponent>();
    view.each([&mouse_pos_adjusted_in_worldspace, &GRID_SIZE](const auto entity, const auto& c, auto& pos) {
      glm::ivec2 grid_slot = engine::grid::world_space_to_grid_space(mouse_pos_adjusted_in_worldspace, GRID_SIZE);
      // ImGui::Text("mouse grid %i %i", grid_slot.x, grid_slot.y);

      glm::ivec2 world_space = grid_slot * GRID_SIZE;
      pos.x = grid_slot.x * GRID_SIZE;
      pos.y = grid_slot.y * GRID_SIZE;
      // ImGui::Text("mouse clamped %i %i", world_space.x, world_space.y);
    });
  }
};
