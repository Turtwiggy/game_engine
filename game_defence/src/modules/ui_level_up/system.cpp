#include "system.hpp"

#include "components.hpp"
#include "entt/helpers.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/renderer/components.hpp"

#include "imgui.h"

namespace game2d {

void
update_ui_level_up_system(entt::registry& r)
{
  const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
  auto& levelup = get_first_component<SINGLE_UILevelUpComponent>(r);

  // HACK: test leveling up window.
  auto& first_player = get_first_component<PlayerComponent>(r);
  if (first_player.picked_up_xp >= 10)
    levelup.show_menu = true;

  if (!levelup.show_menu)
    return;

  const auto& viewport_pos = ImVec2(ri.viewport_pos.x, ri.viewport_pos.y);
  const auto& viewport_size_half = ImVec2(ri.viewport_size_current.x * 0.5f, ri.viewport_size_current.y * 0.5f);
  const auto pos = ImVec2(viewport_pos.x + viewport_size_half.x, viewport_pos.y + viewport_size_half.y);
  ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
  ImGui::SetNextWindowSizeConstraints(ImVec2(640, 640 * (9 / 16.0f)), ImVec2(640, 640 * (9 / 16.0f)));

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoMove;
  flags |= ImGuiWindowFlags_NoCollapse;
  flags |= ImGuiWindowFlags_NoTitleBar;
  // flags |= ImGuiWindowFlags_NoBackground;

  ImGui::Begin("Level Up", &levelup.show_menu, flags);

  if (ImGui::BeginTable("level-up-table", 3)) {
    ImGui::TableNextRow();

    ImGui::TableNextColumn();
    if (ImGui::Button("GAIN DAMAGE\n+1 Damage", ImVec2(-FLT_MIN, -FLT_MIN))) {

      // HACK
      first_player.picked_up_xp -= 10;
      levelup.show_menu = false;
    }

    ImGui::TableNextColumn();
    if (ImGui::Button("GAIN FIRERATE\n+0.01 Firerate", ImVec2(-FLT_MIN, -FLT_MIN))) {

      // HACK
      first_player.picked_up_xp -= 10;
      levelup.show_menu = false;
    }

    ImGui::TableNextColumn();
    if (ImGui::Button("GAIN UNIT\n+1 Unit", ImVec2(-FLT_MIN, -FLT_MIN))) {

      // HACK
      first_player.picked_up_xp -= 10;
      levelup.show_menu = false;
    }

    ImGui::EndTable();
  }
  ImGui::End();
}

} // namespace game2d