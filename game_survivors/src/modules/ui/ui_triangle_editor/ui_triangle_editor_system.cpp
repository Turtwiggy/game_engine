#include "pch.hpp"

#include "ui_triangle_editor_system.hpp"

#include "engine/colour/colour.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/imgui/helpers.hpp"
#include "engine/sprites/components.hpp"
#include "modules/ui/ui_debug_menubar/ui_debug_menubar_components.hpp"
#include "modules/ui/ui_debug_menubar/ui_debug_menubar_helpers.hpp"

namespace game2d {

void
update_ui_triangle_editor_system(entt::registry& r)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif

  auto& menu_c = get_first_component<SINGLE_DebugMenuBar>(r);
  auto state = gesert_menubar_state(menu_c, "DebugIslandColours");
  if (!state.enabled)
    return;

  // debug island colour
  static engine::SRGBColour island_col{ 35 / 255.0f, 100 / 255.0f, 96 / 255.0f, 1.0f };
  imgui_draw_int("island_col_r", island_col.r);
  imgui_draw_int("island_col_g", island_col.g);
  imgui_draw_int("island_col_b", island_col.b);

  auto view = r.view<SpriteTriangleComponent>();
  for (const auto& [e, sc] : view.each()) {
    // imgui_draw_vec2("pos", sc.a);
    // imgui_draw_vec2("pos_b", sc.b);
    // imgui_draw_vec2("pos_c", sc.c);

    sc.a_colour = engine::SRGBToLinear(island_col);
    sc.b_colour = engine::SRGBToLinear(island_col);
    sc.c_colour = engine::SRGBToLinear(island_col);
  }
}

} // namespace game2d