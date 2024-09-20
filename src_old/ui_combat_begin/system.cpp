#include "system.hpp"

#include "engine/entt/helpers.hpp"
#include "imgui.h"
#include "modules/system_turnbased_enemy/components.hpp"

namespace game2d {

void
update_ui_combat_begin_system(entt::registry& r)
{
  const auto& state = get_first_component<SINGLE_CombatState>(r);
  if (state.team == AvailableTeams::neutral) {
    const auto& viewport = ImGui::GetWindowViewport();
    ImGui::SetNextWindowPos(viewport->GetCenter(), ImGuiCond_Always, ImVec2(0.5, 0.5));
    ImGui::SetNextWindowSize(ImVec2{ 200, 100 });

    ImGuiWindowFlags flags = 0;
    flags |= ImGuiWindowFlags_NoFocusOnAppearing;
    flags |= ImGuiWindowFlags_NoTitleBar;
    flags |= ImGuiWindowFlags_NoCollapse;
    flags |= ImGuiWindowFlags_NoResize;
    flags |= ImGuiWindowFlags_NoBackground;
    flags |= ImGuiDockNodeFlags_NoResize;
    flags |= ImGuiDockNodeFlags_PassthruCentralNode;

    ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, { 0.5f, 0.5f });
    ImGui::Begin("Fight!", NULL, flags);

    // Center button horizontally
    // ImGuiStyle& style = ImGui::GetStyle();

    // calculate reqiured x-spacing
    // float width = 0.0f;
    // width += ImGui::CalcTextSize(label.c_str()).x;
    // width += style.ItemSpacing.x;

    // AlignForWidth()
    // const float alignment = 0.5f;
    // const float avail = ImGui::GetContentRegionAvail().x;
    // float off = (avail - width) * alignment;
    // if (off > 0.0f)
    //   ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);

    // Draw the button(s)
    const ImVec2 button_size = ImGui::GetContentRegionAvail();
    if (ImGui::Button("Begin Combat", button_size)) {
      auto& state = get_first_component<SINGLE_CombatState>(r);
      state.team = AvailableTeams::player;
    }

    // HACK: hide this menu
    // state.team = AvailableTeams::player;

    ImGui::End();
    ImGui::PopStyleVar();
  }
}

} // namespace game2d