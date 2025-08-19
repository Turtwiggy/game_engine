#include "pch.hpp"

#include "element_cursor_helpers.hpp"
#include "resources/data.hpp"

namespace game2d {

void
draw_cursor(entt::registry& r, UiCursorComponent& cursor_c, const ImVec2 tl, float dt)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif
  auto* draw_list = ImGui::GetWindowDrawList();

  cursor_c.cursor_wiggle_time += dt;
  cursor_c.cursor_wiggle =
    glm::sin(cursor_c.cursor_wiggle_time * cursor_c.cursor_wiggle_frequency) * cursor_c.cursor_wiggle_amplitude;

  // ImGui::Begin("DebugCursorSize");
  // imgui_draw_int("cursor_size", cursor_size);
  // imgui_draw_vec2("cursor_offset_base", cursor_offset_base);
  // imgui_draw_vec2("cursor_offset", cursor_offset);
  // imgui_draw_float("cursor_wiggle", cursor_wiggle);
  // imgui_draw_float("cursor_wiggle_frequency", cursor_wiggle_frequency);
  // imgui_draw_float("cursor_wiggle_amplitude", cursor_wiggle_amplitude);
  // ImGui::End();
  cursor_c.cursor_offset = cursor_c.cursor_offset_base;
  cursor_c.cursor_offset.y += cursor_c.cursor_wiggle;

  float x_pct = tl.x;
  float y_pct = tl.y;

  const auto cursor_offset = cursor_c.cursor_offset;
  const auto cursor_size = cursor_c.cursor_size;
  const auto im_cursor_col = ImColor{ 1.0f, 0.3f, 0.3f, 1.0f };

  // draw cursor as a down arrow
  draw_list->AddTriangleFilled(ImVec2(cursor_offset.x + x_pct, y_pct + cursor_offset.y),
                               ImVec2(cursor_offset.x + x_pct + 0.5f * cursor_size, y_pct + cursor_size + cursor_offset.y),
                               ImVec2(cursor_offset.x + x_pct + cursor_size, y_pct + cursor_offset.y),
                               im_cursor_col);
  draw_list->AddTriangle(ImVec2(cursor_offset.x + x_pct, y_pct + cursor_offset.y),
                         ImVec2(cursor_offset.x + x_pct + 0.5f * cursor_size, y_pct + cursor_size + cursor_offset.y),
                         ImVec2(cursor_offset.x + x_pct + cursor_size, y_pct + cursor_offset.y),
                         im_white,
                         0.5f);
}

} // namespace game2d