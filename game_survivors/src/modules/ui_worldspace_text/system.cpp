#include "pch.hpp"

#include "modules/ui_worldspace_text/system.hpp"

#include "components.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/core_camera/helpers.hpp"

namespace game2d {
using namespace std::literals;

void
update_ui_worldspace_text_system(entt::registry& r)
{
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 0.0f, 0.0f });

  const auto& view = r.view<const TransformComponent, const WorldspaceTextComponent>();
  for (const auto& [e, t_c, wst_c] : view.each()) {
    if (!wst_c.display)
      continue;
    const auto eid = static_cast<uint32_t>(e);

    const auto wsp = glm::vec2(t_c.position.x, t_c.position.y);
    const auto wsp_adj = glm::vec2{ wsp.x + wst_c.offset.x, wsp.y + wst_c.offset.y };
    const auto ss_pos = worldspace_to_screenspace(r, wsp_adj);

    ImGui::SetNextWindowPos(ImVec2{ ss_pos.x, ss_pos.y }, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(wst_c.size, ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(wst_c.alpha);

    std::string beginlabel = "WorldspaceText##"s + std::to_string(eid);

    ImGuiWindowFlags flags = 0;
    flags |= wst_c.flags;
    if (wst_c.size.x == 0.0f && wst_c.size.y == 0.0f)
      flags |= ImGuiWindowFlags_AlwaysAutoResize;

    ImGui::Begin(beginlabel.c_str(), NULL, flags);
    ImGui::PushID(eid);

    wst_c.layout(r); // layout set via regular imgui commands

    ImGui::PopID();
    ImGui::End();
  }

  ImGui::PopStyleVar(2);
}

} // namespace game2d