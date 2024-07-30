#include "system.hpp"

#include "entt/helpers.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/scene/components.hpp"
#include "modules/scene/helpers.hpp"
#include "modules/ui_combat_turnbased/components.hpp"

#include "imgui.h"
#include "modules/ui_event_console/components.hpp"

namespace game2d {

void
update_ui_combat_ended_system(entt::registry& r)
{
  // check if all enemies are ded.
  std::map<AvailableTeams, int> team_count;
  team_count[AvailableTeams::player] = 0; // force team to exist
  team_count[AvailableTeams::enemy] = 0;  // force team to exist
  for (const auto& [e, team_c] : r.view<TeamComponent>().each())
    team_count[team_c.team] += 1; // count teams
  if (team_count[AvailableTeams::enemy] == 0) {
    const bool you_came_here_from_overworld = get_first<OverworldToDungeonInfo>(r) != entt::null;
    if (you_came_here_from_overworld) {

      // center this window
      const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
      const auto& viewport_pos = ImVec2(ri.viewport_pos.x, ri.viewport_pos.y);
      const auto& viewport_size_half = ImVec2(ri.viewport_size_current.x * 0.5f, ri.viewport_size_current.y * 0.5f);
      const auto pos = ImVec2(viewport_pos.x + viewport_size_half.x, viewport_pos.y + viewport_size_half.y);
      ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
      ImGui::SetNextWindowSizeConstraints(ImVec2(200, 200 * (9 / 16.0f)), ImVec2(200, 200 * (9 / 16.0f)));

      ImGuiWindowFlags flags = 0;
      // position and sizing
      flags |= ImGuiWindowFlags_NoMove;
      flags |= ImGuiWindowFlags_NoCollapse;
      flags |= ImGuiWindowFlags_NoDocking;
      flags |= ImGuiWindowFlags_NoResize;
      flags |= ImGuiWindowFlags_NoFocusOnAppearing;
      // visuals
      flags |= ImGuiWindowFlags_NoTitleBar;
      flags |= ImGuiWindowFlags_NoBackground;

      ImGui::Begin("Back To Overworld", NULL, flags);

      // note: this should not be gameover, but back to
      // some sort of ui that would let you pick more fights.

      if (ImGui::Button("Gameover. Back to menu.", ImVec2(-FLT_MIN, -FLT_MIN))) {
        ImGui::End();

        // add an event because fun
        auto& evt = get_first_component<SINGLE_EventConsoleLogComponent>(r);
        evt.events.push_back("Spaceship cleared.");
        move_to_scene_start(r, Scene::menu, false);
        return;
      }
      ImGui::End();
    } else {
      // Trying to leave the combat scene, but you probably launched standalone..
    }
  }
}

} // namespace game2d