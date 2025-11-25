#include "pch.hpp"

#include "resources/data.hpp"
#include "ui_land_on_island_popup_helpers.hpp"
#include "ui_land_on_island_popup_system.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/imgui/ui_imgui_defaults.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/actors/actor_rock/rock_components.hpp"
#include "modules/core/camera/helpers.hpp"
#include "modules/core/fonts/fonts_helpers.hpp"
#include "modules/core/renderer/components.hpp"
#include "modules/core/ui/ui_common_components.hpp"
#include "modules/steam_input/steam_input_components.hpp"
#include "modules/systems/system_island_movement/island_movement_components.hpp"
#include "modules/systems/system_island_nearest/island_nearest_components.hpp"
#include "modules/ui/ui_colours/ui_colours_helpers.hpp"
#include "modules/ui/ui_scene_main_menu_controllerinfo/ui_main_menu_controllerinfo_helpers.hpp"
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
  const auto font_size = (float)FontSizes::SIZE_13;
  auto* font = get_inter_font(r);
  ImGui::PushFont(font, font_size);

  const auto& ri = SINGLE_RendererInfo::instance;

  const auto screen_size = ImVec2{ (float)ri.viewport_size_render_at.x, (float)ri.viewport_size_render_at.y };
  ImGui::SetNextWindowPos({ 0, 0 }, ImGuiCond_Always, { 0.0f, 0.0f });
  ImGui::SetNextWindowSize(screen_size, ImGuiCond_Always);

  imgui_begin("overlay-island-land-popup", ImGuiWindowFlags_NoInputs);

  //
  // buttons seem to periodically revert to being "..." when polled
  // dont want to display this "...",
  // so take a cache of the value it was when it wasnt "..."
  //
  for (const auto& [e, player_c] : r.view<PlayerComponent>().each()) {
    const auto handle = steam_c.handles[player_c.idx];
    if (player_c.back_button_cached == "") {
      if (handle != 0) {
        auto str_east = get_str_for_da(steam_c, handle, DigitalAction::Game_East);
        if (str_east != "...")
          player_c.back_button_cached = str_east;
      }
    }
    if (player_c.confirm_button_cached == "") {
      if (handle != 0) {
        auto button_str = get_str_for_da(steam_c, handle, DigitalAction::Game_South);
        if (button_str != "...")
          player_c.confirm_button_cached = button_str;
      }
    }
  }

  //
  // Display "press x to land" popup
  //
  {
    auto view = r.view<const IslandNearestComponent, const PlayerComponent, const TransformComponent>();
    for (const auto& [e, nearest_c, player_c, t_c] : view.each()) {
      if (nearest_c.landable_positions.empty())
        continue; // not interested.
      ImGui::PushID(static_cast<uint32_t>(e));

      const auto font_size = (float)FontSizes::SIZE_13;
      ImGui::PushFont(font, font_size);

      auto confirm_button_str = player_c.confirm_button_cached;
      if (confirm_button_str == "")
        confirm_button_str = "Enter";

      const auto label = std::format("({}) Land", player_c.confirm_button_cached);
      const auto label_size = ImGui::CalcTextSize(label.c_str());
      ImGui::PopFont();

      const auto boat_pos = get_position(r, e);
      const auto boat_size = glm::vec2{ t_c.scale.x, t_c.scale.y };

      auto offset = glm::vec2{ 0, 0 };
      offset.x -= 0.5f * label_size.x;
      offset.y -= 1.5f * boat_size.y;

      const auto wsp = boat_pos + offset;
      const auto wsp_ss = worldspace_to_screenspace(r, wsp);
      const auto ss_pos_tl = ImVec2(wsp_ss.x, wsp_ss.y);

      const auto border_col = default_player_colours_im[player_c.colour_idx];
      draw_popup(r, ss_pos_tl, label, font, font_size, border_col);

      ImGui::PopID();
    }
  }

  //
  // Display "press x to return to boat" (on your boat) popup
  //
  {
    const auto view = r.view<const MovementIslandComponent, const TransformComponent, const InputComponent>();
    for (const auto [e, movement_c, t_c, input_c] : view.each()) {
      auto boat_e = movement_c.boat_e;
      if (boat_e == entt::null || !r.valid(boat_e))
        continue;
      ImGui::PushID(static_cast<uint32_t>(e));

      auto& player_c = r.get<PlayerComponent>(boat_e);
      const auto player_idx = player_c.idx;
      const auto player_col_idx = player_c.colour_idx;
      const auto handle = steam_c.handles[player_idx];

      auto back_button = player_c.back_button_cached;
      if (back_button == "")
        back_button = "ESC";

      ImGui::PushFont(font, font_size);
      const auto str0 = "(DPAD) Act";
      const auto str1 = std::format("({}) Embark", back_button);
      const auto str0_len = ImGui::CalcTextSize(str0);
      const auto str1_len = ImGui::CalcTextSize(str1.c_str());
      ImGui::PopFont();

      const auto& boat_tc = r.get<const TransformComponent>(boat_e);
      const auto boat_pos = glm::vec2{ boat_tc.position.x, boat_tc.position.y };
      const auto boat_size = glm::vec2{ boat_tc.scale.x, boat_tc.scale.y };

      auto offset = glm::vec2{ 0, 0 };
      offset.y -= 1.5f * boat_size.y;

      const auto wsp = boat_pos + offset;
      const auto wsp_ss = worldspace_to_screenspace(r, wsp);
      const auto ss_pos_tl = ImVec2(wsp_ss.x, wsp_ss.y);

      const auto ss_pos = ss_pos_tl;
      const auto border_col = default_player_colours_im[player_col_idx];

      const float max = glm::max(str0_len.x, str1_len.x);
      draw_popup(r, ss_pos + ImVec2{ -0.5f * max, -str0_len.y }, str0, font, font_size, border_col);
      draw_popup(r, ss_pos + ImVec2{ -0.5f * max, 0.0f }, str1, font, font_size, border_col);

      ImGui::PopID();
    }
  }

  ImGui::End();
  ImGui::PopFont();
}

} // namespace game2d