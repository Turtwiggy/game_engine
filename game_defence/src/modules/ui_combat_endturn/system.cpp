#include "system.hpp"

#include "entt/helpers.hpp"
#include "modules/renderer/components.hpp"
#include "modules/system_turnbased_enemy/components.hpp"

#include "imgui.h"

namespace game2d {

void
update_ui_combat_endturn_system(entt::registry& r)
{
  const auto state_e = get_first<SINGLE_CombatState>(r);
  if (state_e == entt::null)
    return;
  const auto& state = get_first_component<SINGLE_CombatState>(r);
  // const auto type_name = std::string(magic_enum::enum_name(state.team));

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
  const ImVec2 size = { 100, 100 * 9 / 16.0f };
  ImGui::SetNextWindowSize(size, ImGuiCond_Always);

  // position
  const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
  const auto& viewport_pos = ImVec2(ri.viewport_pos.x, ri.viewport_pos.y);
  const auto pos = ImVec2(viewport_pos.x + ri.viewport_size_current.x - size.x, //
                          viewport_pos.y + ri.viewport_size_current.y - size.y);
  ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

  ImGui::Begin("UI Combat EndTurn", NULL, flags);
  {
    if (state.team == AvailableTeams::player) {
      if (ImGui::Button("End Turn", ImVec2(-FLT_MIN, -FLT_MIN))) // let player end their turn
        create_empty<RequestToCompleteTurn>(r, RequestToCompleteTurn{ AvailableTeams::player });
    }
  }
  ImGui::End();
}

} // namespace game2d