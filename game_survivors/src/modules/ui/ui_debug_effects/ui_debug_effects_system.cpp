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
  auto& menu_c = get_first_component<SINGLE_DebugMenuBar>(r);

  const auto effects_ui = gesert_menubar_state(menu_c, "DebugEffects");
  if (!effects_ui.enabled)
    return;

  imgui_begin("DebugEffects");

  if (ImGui::Button("Explosion0"))
    spawn_fx(r, "EXPLODE_FX_0", { 0, 0 }, { 128, 128 });

  if (ImGui::Button("Explosion1"))
    spawn_fx(r, "EXPLODE_FX_1", { 0, 0 }, { 128, 128 });

  if (ImGui::Button("Block0"))
    spawn_fx(r, "BLOCK_FX_0", { 0, 0 }, { 128, 128 });

  if (ImGui::Button("Block1"))
    spawn_fx(r, "BLOCK_FX_1", { 0, 0 }, { 128, 128 });

  ImGui::End();
}

} // namespace game2d