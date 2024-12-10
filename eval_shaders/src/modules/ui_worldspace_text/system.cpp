#include "modules/ui_worldspace_text/system.hpp"

#include "components.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/renderer/transform.hpp"
#include "imgui.h"
#include "modules/camera/orthographic.hpp"
#include "modules/renderer/components.hpp"

namespace game2d {
using namespace std::literals;

void
update_ui_worldspace_text_system(entt::registry& r)
{
  const auto& ri = get_first_component<SINGLE_RendererInfo>(r);

  const auto camera_e = get_first<OrthographicCamera>(r);
  const auto& camera_c = r.get<OrthographicCamera>(camera_e);
  const auto& camera_t = r.get<TransformComponent>(camera_e);
  const auto zoom = camera_c.zoom_nonlinear;
  const ImVec2 zoom_vec = { zoom, zoom };

  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 4.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 4.0f, 4.0f });
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 0.0f, 0.0f });

  const auto& view = r.view<TransformComponent, WorldspaceTextComponent>();
  for (const auto& [e, t, wst_c] : view.each()) {
    const auto eid = static_cast<uint32_t>(e);

    const auto t_pos = glm::vec2(t.position.x, t.position.y);
    // const auto worldspace = position_in_worldspace(r, t_pos);

    // worldspace adjusted...
    const ImVec2 worldspace_adj = { static_cast<float>(t_pos.x) + wst_c.offset.x,
                                    static_cast<float>(t_pos.y) + wst_c.offset.y };

    const auto screen_center = ImVec2{ ri.viewport_size_render_at.x / 2.0f, ri.viewport_size_render_at.y / 2.0f };
    const auto target_pos = glm::vec2{ worldspace_adj.x, worldspace_adj.y };
    const auto camera_pos = glm::vec2{ camera_t.position.x, camera_t.position.y };
    const auto dir = (target_pos - camera_pos) / zoom;
    const auto pos = ImVec2{ screen_center.x + dir.x, screen_center.y + dir.y };

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

  ImGui::PopStyleVar(4);
}

} // namespace game2d