#include "pch.hpp"

#include "ui_debug_effects_system.hpp"

#include "effects_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/imgui/helpers.hpp"
#include "engine/imgui/ui_imgui_defaults.hpp"
#include "modules/ui/ui_debug_menubar/ui_debug_menubar_helpers.hpp"

namespace game2d {

void
update_ui_debug_effects_system(entt::registry& r)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif
  auto& menu_c = get_first_component<SINGLE_DebugMenuBar>(r);

  const auto effects_ui = gesert_menubar_state(menu_c, "DebugEffects");
  if (!effects_ui.enabled)
    return;

  imgui_begin("DebugEffects");

  static float sprite_fps = 24;
  imgui_draw_float("sprite_fps", sprite_fps);

  if (ImGui::Button("EXPLODE_FX_6"))
    spawn_fx(r, "EXPLODE_FX_6", { 0, 0 }, { 128, 128 }, sprite_fps);

  if (ImGui::Button("EXPLODE_FX_7"))
    spawn_fx(r, "EXPLODE_FX_7", { 0, 0 }, { 128, 128 }, sprite_fps);

  if (ImGui::Button("BLOCK_FX_0"))
    spawn_fx(r, "BLOCK_FX_0", { 0, 0 }, { 128, 128 }, sprite_fps);

  if (ImGui::Button("BLOCK_FX_1"))
    spawn_fx(r, "BLOCK_FX_1", { 0, 0 }, { 128, 128 }, sprite_fps);

  if (ImGui::Button("S6_EXPLODE_FX_7"))
    spawn_fx(r, "S6_EXPLODE_FX_7", { 0, 0 }, { 128, 128 }, sprite_fps);

  if (ImGui::Button("S6_EXPLODE_FX_14"))
    spawn_fx(r, "S6_EXPLODE_FX_14", { 0, 0 }, { 128, 128 }, sprite_fps);

  if (ImGui::Button("S6_EXPLODE_FX_21"))
    spawn_fx(r, "S6_EXPLODE_FX_21", { 0, 0 }, { 128, 128 }, sprite_fps);

  ImGui::End();
}

} // namespace game2d