#include "pch.hpp"

#include "resources/data.hpp"
#include "ui_land_on_island_popup_helpers.hpp"
#include "ui_land_on_island_popup_system.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/core/camera/helpers.hpp"
#include "modules/core/fonts/fonts_helpers.hpp"
#include "modules/core/renderer/components.hpp"
#include "modules/core/ui/ui_common_components.hpp"
#include "modules/steam_input/steam_input_components.hpp"
#include "modules/systems/system_island_movement/island_movement_components.hpp"
#include "modules/systems/system_island_nearest/island_nearest_components.hpp"
#include "modules/ui/ui_colours/ui_colours_helpers.hpp"
#include "modules/ui/ui_scene_survive_onboarding/ui_survive_onboarding_helpers.hpp"

namespace game2d {

void
update_ui_land_on_island_popup_system(entt::registry& r)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif
  const auto& steam_c = get_first_component<SINGLE_SteamControllers>(r);

  const auto font_scale = get_first_component<SINGLE_UIScaling>(r).scaling;
  const auto font_enum = font_scale == 1.0f ? FontSize::TEXT_SIZE_16 : FontSize::TEXT_SIZE_16_SCALED;
  auto* font = get_inter_font(r, font_enum);
  ImGui::PushFont(font);

  const auto& ri = SINGLE_RendererInfo::instance;
  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoDecoration;
  flags |= ImGuiWindowFlags_NoMove;
  flags |= ImGuiWindowFlags_NoFocusOnAppearing;
  flags |= ImGuiWindowFlags_NoDocking;
  flags |= ImGuiWindowFlags_NoInputs;
  flags |= ImGuiWindowFlags_NoSavedSettings;
  flags |= ImGuiWindowFlags_NoBackground;

  const auto screen_size = ImVec2{ (float)ri.viewport_size_render_at.x, (float)ri.viewport_size_render_at.y };
  ImGui::SetNextWindowPos({ 0, 0 }, ImGuiCond_Always, { 0.0f, 0.0f });
  ImGui::SetNextWindowSize(screen_size, ImGuiCond_Always);

  ImGui::Begin("overlay-island-land-popup", NULL, flags);

  //
  // Display "press x to land" popup
  //
  if (false) {
    auto view = r.view<const IslandNearestComponent, const PlayerComponent>();
    for (const auto& [e, nearest_c, player_c] : view.each()) {
      if (nearest_c.landable_positions.empty())
        continue; // not interested.
      ImGui::PushID(static_cast<uint32_t>(e));

      const auto handle = steam_c.handles[player_c.idx];
      const auto confirm_button_str = get_confirm_button_str(r, handle);
      const auto label = std::format("Press {} to land", confirm_button_str);

      const auto wsp = nearest_c.landable_positions[0];
      const auto wsp_ss = worldspace_to_screenspace(r, wsp);
      const auto ss_pos_tl = ImVec2(wsp_ss.x, wsp_ss.y);

      const auto border_col = default_player_colours_im[player_c.idx];
      draw_popup(r, ss_pos_tl, label, font, border_col);

      ImGui::PopID();
    }
  }

  //
  // Display "press x to return to boat" (on your boat) popup
  //
  if (false) {
    const auto view = r.view<const MovementIslandComponent, const TransformComponent, const InputComponent>();
    for (const auto [e, movement_c, t_c, input_c] : view.each()) {
      auto boat_e = movement_c.boat_e;
      if (boat_e == entt::null || !r.valid(boat_e))
        continue;
      ImGui::PushID(static_cast<uint32_t>(e));

      const auto boat_pos = get_position(r, boat_e);
      const auto wsp_ss = worldspace_to_screenspace(r, boat_pos);
      const auto ss_pos_tl = ImVec2(wsp_ss.x, wsp_ss.y);

      const auto player_idx = r.get<PlayerComponent>(boat_e).idx;
      const auto handle = steam_c.handles[0];
      const auto confirm_button_str = get_back_button_str(r, handle);
      const auto label = std::format("Use DPAD (move/attack)\nPress {} - return to boat", confirm_button_str);
      const auto label_wh = ImGui::CalcTextSize(label.c_str());
      const auto ss_pos = ss_pos_tl + ImVec2{ -0.5f * label_wh.x, 0.5f * label_wh.y };

      const auto border_col = default_player_colours_im[player_idx];
      draw_popup(r, ss_pos, label, font, border_col);

      ImGui::PopID();
    }
  }

  ImGui::End();
  ImGui::PopFont();
}

} // namespace game2d