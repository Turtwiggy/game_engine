#include "system.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/events/components.hpp"
#include "engine/events/helpers/keyboard.hpp"
#include "modules/actor_cargo/helpers.hpp"
#include "modules/combat_damage/components.hpp"
#include "modules/gameover/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/scene/components.hpp"
#include "modules/scene/helpers.hpp"
#include "modules/ui_event_console/components.hpp"

#include "imgui.h"
#include <cstddef>

namespace game2d {

void
update_ui_combat_ended_system(entt::registry& r)
{
  // check if all enemies are ded.
  std::map<AvailableTeams, int> team_count;
  team_count[AvailableTeams::player] = 0; // force team to exist
  team_count[AvailableTeams::enemy] = 0;  // force team to exist

  const auto view = r.view<TeamComponent>();
  for (const auto& [e, team_c] : view.each())
    team_count[team_c.team] += 1; // count teams

  bool all_enemies_ded = team_count[AvailableTeams::enemy] == 0;
  // bool you_came_here_from_overworld = get_first<OverworldToDungeonInfo>(r) != entt::null;

#if defined(_DEBUG)
  const auto& input = get_first_component<SINGLE_InputComponent>(r);
  if (get_key_held(input, SDL_SCANCODE_KP_9))
    all_enemies_ded = true;
  if (get_key_down(input, SDL_SCANCODE_KP_8)) {
    auto& go = get_first_component<SINGLE_GameOver>(r);
    go.game_is_over = true;
    go.win_condition = false;
    go.reason = "Forced gameover; loss";

    // punish the player! make them lose a cargo!
    decrement_cargo(r);
  }

#endif

  if (all_enemies_ded) {

    // center this window
    const auto& ri = get_first_component<SINGLE_RendererInfo>(r);
    const auto viewport_pos = ImVec2((float)ri.viewport_pos.x, (float)ri.viewport_pos.y);
    const auto viewport_size_half = ImVec2(ri.viewport_size_current.x * 0.5f, ri.viewport_size_current.y * 0.5f);
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

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5.0f, 5.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImGui::Begin("Back To Overworld", NULL, flags);

    if (ImGui::Button("Complete.\nReturn to ship", ImVec2(-FLT_MIN, -FLT_MIN))) {
      ImGui::End();
      ImGui::PopStyleVar();
      ImGui::PopStyleVar();
      ImGui::PopStyleVar();

      // add an event because fun
      auto& evt = get_first_component<SINGLE_EventConsoleLogComponent>(r);
      evt.events.push_back("Spaceship cleared.");

      // you gain loot
      increment_cargo(r);

      // note: this should not be gameover, but back to
      // some sort of ui that would let you pick more fights.
      move_to_scene_start(r, Scene::overworld_revamped, false);
      move_to_scene_additive(r, Scene::overworld_revamped);

      return;
    }

    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleVar();
    ImGui::PopStyleVar();
  }
}

} // namespace game2d