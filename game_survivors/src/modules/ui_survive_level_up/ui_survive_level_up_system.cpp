#include "modules/ui_survive_level_up/ui_survive_level_up_system.hpp"

#include "engine/entt/helpers.hpp"
#include "imgui.h"
#include "modules/event_coll_player_xp/event_coll_player_xp_components.hpp"
#include "modules/renderer/components.hpp"

namespace game2d {

void
update_ui_survive_level_up_system(entt::registry& r)
{
  GET_FIRST_OR_RETURN(SINGLE_XpComponent, r, sxp_e, sxp_c);
  GET_FIRST_OR_RETURN(SINGLE_RendererInfo, r, ri_e, ri);

  if (sxp_c.xp < sxp_c.xp_for_next_level) {
#if defined(_DEBUG)
    if (ImGui::Button("GiveXp"))
      sxp_c.xp += 34;
#endif
    return;
  }

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoDecoration;
  flags |= ImGuiWindowFlags_NoMove;
  flags |= ImGuiWindowFlags_NoBackground;
  flags |= ImGuiWindowFlags_NoDocking;
  flags |= ImGuiWindowFlags_NoFocusOnAppearing;

  const auto& viewport_pos = ImVec2(ri.viewport_pos.x, ri.viewport_pos.y);
  const auto& viewport_size_half = ImVec2(ri.viewport_size_current.x * 0.5f, ri.viewport_size_current.y * 0.5f);
  const auto pos = ImVec2(viewport_pos.x + viewport_size_half.x, viewport_pos.y + viewport_size_half.y);
  ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
  ImGui::SetNextWindowSize({ 640, 360 }, ImGuiCond_Always);

  ImGui::Begin("Level up required!", NULL, flags);
  ImGui::Text("Levelup required");

  std::string label = std::format("LeveUp");

  if (ImGui::BeginTable(label.c_str(), 3)) {
    ImGui::TableNextRow();

    ImGui::TableNextColumn();
    if (ImGui::Button("con\n+4 MAX HP", ImVec2(-FLT_MIN, -FLT_MIN))) {
      sxp_c.xp = 0;

      //
    }

    ImGui::TableNextColumn();
    if (ImGui::Button("str\n+1 ATK", ImVec2(-FLT_MIN, -FLT_MIN))) {
      sxp_c.xp = 0;

      //
    }

    ImGui::TableNextColumn();
    if (ImGui::Button("agi\n+1 DEF", ImVec2(-FLT_MIN, -FLT_MIN))) {
      sxp_c.xp = 0;

      //
    }

    ImGui::EndTable();
  }

  ImGui::End();
}

} // namespace game2d