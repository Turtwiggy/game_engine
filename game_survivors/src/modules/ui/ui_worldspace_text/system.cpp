#include "pch.hpp"

#include "modules/ui/ui_worldspace_text/system.hpp"

#include "components.hpp"
#include "engine/renderer/transform.hpp"
#include "modules/core/camera/helpers.hpp"
#include "modules/core/renderer/components.hpp"

namespace game2d {
using namespace std::literals;

void
update_ui_worldspace_text_system(entt::registry& r)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif
  const auto& ri = SINGLE_RendererInfo::instance;

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoDecoration;
  flags |= ImGuiWindowFlags_NoMove;
  flags |= ImGuiWindowFlags_NoFocusOnAppearing;
  flags |= ImGuiWindowFlags_NoDocking;
  flags |= ImGuiWindowFlags_NoInputs;
  flags |= ImGuiWindowFlags_NoSavedSettings;
  flags |= ImGuiWindowFlags_NoBackground;

  const auto screen_size = ImVec2{ (float)ri.viewport_size_render_at.x, (float)ri.viewport_size_render_at.y };
  ImGui::SetNextWindowPos({ 0, 0 }, ImGuiCond_Always, { 0.0f, 0.0f });
  ImGui::SetNextWindowSize(screen_size, ImGuiCond_Always);
  ImGui::Begin("overlay", NULL, flags);

  const auto view = r.view<const TransformComponent, const WorldspaceTextComponent>();
  for (const auto& [e, t_c, wst_c] : view.each()) {
    const auto eid = static_cast<uint32_t>(e);
    ImGui::PushID(eid);

    const auto wsp = glm::vec2(t_c.position.x, t_c.position.y);
    const auto wsp_adj = glm::vec2{ wsp.x + wst_c.offset.x, wsp.y + wst_c.offset.y };
    const auto ss_pos = worldspace_to_screenspace(r, wsp_adj);
    ImGui::SetCursorScreenPos({ ss_pos.x, ss_pos.y });

    wst_c.layout(r, e, wst_c); // layout set via regular imgui commands

    ImGui::PopID();
  }

  ImGui::End();
}

} // namespace game2d