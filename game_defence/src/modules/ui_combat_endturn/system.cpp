#include "system.hpp"

#include "entt/helpers.hpp"
#include "events/components.hpp"
#include "events/helpers/keyboard.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/renderer/components.hpp"
#include "modules/system_turnbased_enemy/components.hpp"

#include "fmt/core.h"
#include "imgui.h"
#include "magic_enum.hpp"

namespace game2d {

void
update_ui_combat_endturn_system(entt::registry& r)
{
  const auto state_e = get_first<SINGLE_CombatState>(r);
  if (state_e == entt::null)
    return;
  const auto& state = get_first_component<SINGLE_CombatState>(r);
  const auto team_name = std::string(magic_enum::enum_name(state.team));

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

  // window settings

  // size
  const ImVec2 size = { 200, 60 };
  ImGui::SetNextWindowSize(size, ImGuiCond_Always);

  // position
  const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
  const auto viewport_pos = ImVec2((float)ri.viewport_pos.x, (float)ri.viewport_pos.y);
  const auto pos = ImVec2(viewport_pos.x + ri.viewport_size_current.x - (size.x / 2.0f), //
                          viewport_pos.y + ri.viewport_size_current.y - (size.y / 2.0f));
  ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

  ImGui::Begin("UI Combat EndTurn", NULL, flags);
  {
    const std::string turn_label = fmt::format("Turn: {}", team_name);
    ImGui::Text(turn_label.c_str(), ImVec2(-FLT_MIN, -FLT_MIN));

    if (state.team == AvailableTeams::player) {
      const auto& view = r.view<PlayerComponent, ActionState>();
      for (const auto& [e, player, actions] : view.each()) {
        const std::string label = fmt::format("Actions: {}", actions.actions_available);
        ImGui::Text(label.c_str(), ImVec2(-FLT_MIN, -FLT_MIN));
      }
    }
  }
  ImGui::End();
}

} // namespace game2d