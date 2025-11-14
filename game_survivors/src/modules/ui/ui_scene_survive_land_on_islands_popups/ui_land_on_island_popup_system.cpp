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

      // const auto handle = steam_c.handles[player_c.idx];
      // const auto confirm_button_str = get_confirm_button_str(r, handle);
      const auto label = std::string("(Confirm) Land");
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

      const auto border_col = default_player_colours_im[player_c.idx];
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

      ImGui::PushFont(font, font_size);
      const auto str0 = "(DPAD) Move & Attack";
      const auto str1 = "(Back) Embark";
      const auto str0_len = ImGui::CalcTextSize(str0);
      const auto str1_len = ImGui::CalcTextSize(str1);
      ImGui::PopFont();

      const auto& boat_tc = r.get<const TransformComponent>(boat_e);
      const auto boat_pos = glm::vec2{ boat_tc.position.x, boat_tc.position.y };
      const auto boat_size = glm::vec2{ boat_tc.scale.x, boat_tc.scale.y };

      auto offset = glm::vec2{ 0, 0 };
      offset.y -= 1.5f * boat_size.y;

      const auto wsp = boat_pos + offset;
      const auto wsp_ss = worldspace_to_screenspace(r, wsp);
      const auto ss_pos_tl = ImVec2(wsp_ss.x, wsp_ss.y);

      const auto player_idx = r.get<PlayerComponent>(boat_e).idx;
      // const auto handle = steam_c.handles[0];
      // const auto confirm_button_str = get_back_button_str(r, handle);

      const auto ss_pos = ss_pos_tl;
      const auto border_col = default_player_colours_im[player_idx];

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