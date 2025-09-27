#include "pch.hpp"

#include "ui_triangle_editor_system.hpp"

#include "engine/imgui/helpers.hpp"
#include "engine/sprites/components.hpp"

namespace game2d {

void
update_ui_triangle_editor_system(entt::registry& r)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif

  auto view = r.view<SpriteTriangleComponent>();
  for (const auto& [e, sc] : view.each()) {
    imgui_draw_vec2("pos", sc.a);
    imgui_draw_vec2("pos_b", sc.b);
    imgui_draw_vec2("pos_c", sc.c);
  }
}

} // namespace game2d