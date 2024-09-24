#include "ui_players_system.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/sprites/components.hpp"
#include "engine/sprites/helpers.hpp"
#include "imgui.h"
#include "modules/actor_player/components.hpp"
#include "modules/camera/components.hpp"
#include "modules/event_damage/event_damage_helpers.hpp"
#include "modules/events/events_components.hpp"
#include "modules/raws/raws_components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/renderer/helpers.hpp"

namespace game2d {

void
update_ui_players_system(entt::registry& r)
{
  const auto& view = r.view<PlayerComponent>();
  const int players = view.size();
  if (players == 0)
    return;

  const auto& ri = get_first_component<SINGLE_RendererInfo>(r);

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoDecoration;

  // Top-Left of the screen
  const auto viewport_pos = ImVec2((float)ri.viewport_pos.x, (float)ri.viewport_pos.y);
  const auto padding = ImVec2(100, 100);
  const auto pos = ImVec2(viewport_pos.x + padding.x, viewport_pos.y + padding.y);
  ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.0f, 0.0f));

  const auto size = ImVec2(64, 64);
  // ImGui::SetNextWindowSize({ size.x, size.y * players });

  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

  ImGui::Begin("Player Selector", NULL, flags);

  for (int i = 0; const auto& [e, player_c] : view.each()) {
    const auto eid = static_cast<uint32_t>(e);
    const auto& mob_c = r.get<Mob>(e);

    // sprite
    const auto tex_id = search_for_texture_id_by_texture_path(ri, "monochrome")->id;
    const ImTextureID im_id = reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(tex_id));
    ImVec2 tl{ 0.0f, 0.0f };
    ImVec2 br{ 1.0f, 1.0f };
    const auto result = convert_sprite_to_uv(r, mob_c.renderable.sprite);
    std::tie(tl, br) = result;

    const std::string label = "player-portrait##" + std::to_string(eid);

    ImGui::ImageButton(label.c_str(), im_id, size, tl, br);

    if (ImGui::IsItemClicked()) {
      fmt::println("clicked portrait...");

      // Remove camera focus from anything else
      // const auto& follow_view = r.view<CameraFollow>();
      // r.destroy(follow_view.begin(), follow_view.end());
      // Add camera follow to a player
      // r.emplace<CameraFollow>(e);
    }

    i++;
  }

  ImGui::End();
  ImGui::PopStyleVar(2);
}

} // namespace game2d