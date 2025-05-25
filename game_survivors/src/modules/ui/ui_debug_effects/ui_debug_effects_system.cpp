#include "pch.hpp"

#include "ui_debug_effects_system.hpp"

#include "effects_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/imgui/helpers.hpp"
#include "engine/imgui/ui_imgui_defaults.hpp"
#include "modules/core/renderer/components.hpp"
#include "modules/systems/system_particles/components.hpp"
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
  const auto vfx_offset = glm::vec2{ 200, 0 };

  static float sprite_fps = 24;
  imgui_draw_float("sprite_fps", sprite_fps);

  if (ImGui::Button("EXPLODE_FX_6"))
    spawn_fx(r, "EXPLODE_FX_6", vfx_offset, { 128, 128 }, sprite_fps);

  if (ImGui::Button("EXPLODE_FX_7"))
    spawn_fx(r, "EXPLODE_FX_7", vfx_offset, { 128, 128 }, sprite_fps);

  if (ImGui::Button("BLOCK_FX_0"))
    spawn_fx(r, "BLOCK_FX_0", vfx_offset, { 128, 128 }, sprite_fps);

  if (ImGui::Button("BLOCK_FX_1"))
    spawn_fx(r, "BLOCK_FX_1", vfx_offset, { 128, 128 }, sprite_fps);

  if (ImGui::Button("S6_EXPLODE_FX_7"))
    spawn_fx(r, "S6_EXPLODE_FX_7", vfx_offset, { 128, 128 }, sprite_fps);

  if (ImGui::Button("S6_EXPLODE_FX_14"))
    spawn_fx(r, "S6_EXPLODE_FX_14", vfx_offset, { 128, 128 }, sprite_fps);

  if (ImGui::Button("S6_EXPLODE_FX_21"))
    spawn_fx(r, "S6_EXPLODE_FX_21", vfx_offset, { 128, 128 }, sprite_fps);

  const auto request_particles = [&](std::string key, float radius = 0) {
    if (ImGui::Button(key.c_str())) {
      RequestToSpawnParticles request;
      request.key = key;
      request.position = vfx_offset;
      request.radius_pixels_upper = radius;
      create_empty<RequestToSpawnParticles>(r, request);
    }
  };
  request_particles("default_explode");
  request_particles("death_sea_mine", 200);
  request_particles("enemy_death");
  request_particles("fire_particles");
  request_particles("death_exploder", 50);
  request_particles("vfx_boop");
  request_particles("vfx_levelup_outer");
  request_particles("vfx_levelup_inner");

  // static bool invert = false;
  // static bool changed = false;
  // auto& ri_c = SINGLE_RendererInfo::instance;
  // auto& invert_c = ri_c.mix_lighting_and_scene;
  // if (ImGui::Button("Invert"))
  //   changed = true;
  // if (changed) {
  //   changed = false;
  //   invert = !invert;
  //   ri_c.mix_lighting_and_scene.bind();
  //   ri_c.mix_lighting_and_scene.set_bool("invert_colours", invert);
  // }

  ImGui::End();
}

} // namespace game2d