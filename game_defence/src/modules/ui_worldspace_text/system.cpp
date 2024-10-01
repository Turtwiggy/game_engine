#include "modules/ui_worldspace_text/system.hpp"

#include "components.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/renderer/transform.hpp"
#include "imgui.h"
#include "modules/camera/helpers.hpp"
#include "modules/renderer/components.hpp"

namespace game2d {
using namespace std::literals;

void
update_ui_worldspace_text_system(entt::registry& r)
{
  const auto& ri = get_first_component<SINGLE_RendererInfo>(r);

  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 4.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 4.0f, 4.0f });

  const auto& view = r.view<TransformComponent, WorldspaceTextComponent>();
  for (const auto& [e, t, wst_c] : view.each()) {
    const auto eid = static_cast<uint32_t>(e);

    const auto t_pos = glm::vec2(t.position.x, t.position.y);
    const auto worldspace = position_in_worldspace(r, t_pos);
    const ImVec2 pos = { static_cast<float>(worldspace.x) + wst_c.offset.x,
                         static_cast<float>(worldspace.y) + wst_c.offset.y };
    ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(wst_c.size, ImGuiCond_Always);

    std::string beginlabel = "WorldspaceText##"s + std::to_string(eid);

    ImGuiWindowFlags flags = 0;
    flags |= wst_c.flags;

    if (wst_c.alpha != 1.0f)
      ImGui::SetNextWindowBgAlpha(wst_c.alpha);

    ImGui::Begin(beginlabel.c_str(), NULL, flags);
    ImGui::PushID(eid);

    wst_c.layout(); // layout set via regular imgui commands

    ImGui::PopID();
    ImGui::End();
  }

  ImGui::PopStyleVar(3);
}

} // namespace game2d