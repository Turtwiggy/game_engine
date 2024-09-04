#include "modules/ui_worldspace_text/system.hpp"

#include "components.hpp"
#include "helpers.hpp"

#include "entt/helpers.hpp"
#include "modules/camera/helpers.hpp"
#include "modules/renderer/components.hpp"
#include "modules/ux_hoverable/components.hpp"
#include "renderer/transform.hpp"

#include <glm/glm.hpp>
#include <imgui.h>

namespace game2d {
using namespace std::literals;

void
update_ui_worldspace_text_system(entt::registry& r)
{
  const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
  // ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0.0f, 0.0f });
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 6);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1);

  const auto& view = r.view<TransformComponent, WorldspaceTextComponent>();
  for (const auto& [e, t, wst_c] : view.each()) {
    const auto eid = static_cast<uint32_t>(e);

    // Does an entity need to be hovered to show this ui?
    const auto is_hovered = r.try_get<HoveredComponent>(e) != nullptr;
    if (wst_c.requires_hovered && !is_hovered)
      continue;

    // Does an entity need to be visible to show this ui?
    // const auto* is_visible = r.try_get<VisibleComponent>(e) != nullptr;
    // if (wst_c.requires_visible && !is_visible)
    //   continue;

    const auto t_pos = glm::ivec2(t.position.x, t.position.y);
    const auto worldspace = position_in_worldspace(r, t_pos);
    ImVec2 pos = { static_cast<float>(worldspace.x), static_cast<float>(worldspace.y) };
    pos.x += wst_c.offset.x;
    pos.y += wst_c.offset.y;

    // keeps in screen boundaries?
    // pos.x = glm::clamp(static_cast<int>(pos.x), 0, ri.viewport_size_render_at.x);
    // pos.y = glm::clamp(static_cast<int>(pos.y), 0, ri.viewport_size_render_at.y);

    ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

    // ImGui::PushStyleColor(ImGuiCol_, (ImVec4)ImColor::HSV(1 / 7.0f, 0.6f, 0.6f));
    ImGuiWindowFlags flags = 0;
    flags |= ImGuiWindowFlags_NoDecoration;
    flags |= ImGuiWindowFlags_NoBackground;
    ImGui::SetNextWindowBgAlpha(0.5); // some background
    flags |= ImGuiDockNodeFlags_AutoHideTabBar;
    flags |= ImGuiDockNodeFlags_NoResize;

    std::string beginlabel = "WorldspaceText##"s + std::to_string(eid);
    ImGui::Begin(beginlabel.c_str(), NULL, flags);
    ImGui::PushID(eid);

    // if (wst_c.font_scale != 1.0f)
    //   ImGui::SetWindowFontScale(wst_c.font_scale);

    if (wst_c.split_text_into_lines) {
      const auto shortened = split_string_nearest_space(wst_c.text, wst_c.line_length);
      for (const auto& line : shortened)
        ImGui::Text("%s", line.c_str());
    } else if (wst_c.layout.has_value())
      wst_c.layout.value()(); // layout set externally via regular imgui commands
    else
      ImGui::Text("%s", wst_c.text.c_str());

    // if (wst_c.font_scale != 1.0f)
    //   ImGui::SetWindowFontScale(1.0f);

    ImGui::PopID();
    ImGui::End();
  }

  ImGui::PopStyleVar(3);
}

} // namespace game2d