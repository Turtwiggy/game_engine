#include "pch.hpp"

#include "ui_land_on_island_popup_system.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/core/camera/helpers.hpp"
#include "modules/core/renderer/components.hpp"
#include "modules/steam_input/steam_input_components.hpp"
#include "modules/systems/system_island_movement/island_movement_components.hpp"
#include "modules/systems/system_island_nearest/island_nearest_components.hpp"
#include "modules/ui/ui_scene_survive_onboarding/ui_survive_onboarding_helpers.hpp"

namespace game2d {

void
update_ui_land_on_island_popup_system(entt::registry& r)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif
  const auto& steam_c = get_first_component<SINGLE_SteamControllers>(r);

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
  {
    auto view = r.view<const IslandNearestComponent, const PlayerComponent>();
    for (const auto& [e, nearest_c, player_c] : view.each()) {
      if (nearest_c.landable_positions.size() == 0)
        continue; // not interested.
      ImGui::PushID(static_cast<uint32_t>(e));

      auto wsp = nearest_c.landable_positions[0];
      const auto ss_pos = worldspace_to_screenspace(r, wsp);
      ImGui::SetCursorScreenPos({ ss_pos.x, ss_pos.y });

      auto handle = steam_c.handles[player_c.idx];
      auto confirm_button_str = get_confirm_button_str(r, handle);
      ImGui::Text("%s", std::format("Press {} (land on island)", confirm_button_str).c_str());

      ImGui::PopID();
      break;
    }
  }

  //
  // Display "press x to return to boat" (on your boat) popup
  //
  {
    const auto view = r.view<const MovementIslandComponent, const TransformComponent, const InputComponent>();
    for (const auto [e, movement_c, t_c, input_c] : view.each()) {
      auto boat_e = movement_c.boat_e;
      if (boat_e == entt::null)
        continue;

      auto boat_pos = get_position(r, boat_e);
      const auto ss_pos = worldspace_to_screenspace(r, boat_pos);

      const auto player_idx = r.get<PlayerComponent>(boat_e).idx;
      const auto handle = steam_c.handles[0];
      const auto confirm_button_str = get_back_button_str(r, handle);
      const auto text_size = ImGui::CalcTextSize("A");

      ImGui::SetCursorScreenPos({ ss_pos.x, ss_pos.y });
      ImGui::Text("%s", std::format("Use DPAD (move)", confirm_button_str).c_str());

      ImGui::SetCursorScreenPos({ ss_pos.x, ss_pos.y + text_size.y });
      ImGui::Text("%s", std::format("Press {} (return to boat)", confirm_button_str).c_str());
    }
  }

  ImGui::End();
}

} // namespace game2d