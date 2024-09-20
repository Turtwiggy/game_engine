#include "swap_active_player.hpp"

#include "modules/actor_player/components.hpp"
#include "modules/camera/components.hpp"

#include "imgui.h"

namespace game2d {

void
update_swap_active_player_system(entt::registry& r)
{
  const auto& view = r.view<PlayerComponent>();
  for (const auto& [e, player_c] : view.each()) {

    const auto* cam_focus = r.try_get<CameraFollow>(e);
    if (cam_focus) {
      if (!r.any_of<InputComponent>(e)) {
        r.emplace<InputComponent>(e);
        r.emplace<KeyboardComponent>(e);
      }
    }

    if (!cam_focus) {
      if (r.any_of<InputComponent, KeyboardComponent>(e)) {
        r.remove<InputComponent>(e);
        r.remove<KeyboardComponent>(e);
      }
    }

    //
  }

// Hack: swap your focus on players
#if defined(_DEBUG)
  ImGui::Begin("Swap player focus");
  static entt::entity newly_focused = entt::null;

  const auto& view_focused = r.view<CameraFollow>();
  const auto& view_non_focused = r.view<PlayerComponent>(entt::exclude<CameraFollow>);

  // Add camera follow to a player
  for (const auto& [e, player_c] : view_non_focused.each()) {
    ImGui::PushID(static_cast<uint32_t>(e));
    if (ImGui::Button("Swap") || view_focused.size() == 0) {
      r.emplace_or_replace<CameraFollow>(e);
      newly_focused = e;
    }
    ImGui::PopID();
  }

  // Remove camera follow from any other player
  const auto& view_player_focused = r.view<PlayerComponent, CameraFollow>();
  for (const auto& [e, player, follow_c] : view_player_focused.each()) {
    if (e == newly_focused)
      continue;
    r.remove<CameraFollow>(e);
  }

  ImGui::End();
#endif
}

} // namespace game2d