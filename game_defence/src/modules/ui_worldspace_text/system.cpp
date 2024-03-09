#include "system.hpp"

#include "components.hpp"
#include "entt/helpers.hpp"
#include "modules/camera/helpers.hpp"
#include "modules/renderer/components.hpp"
#include "renderer/transform.hpp"

#include <glm/glm.hpp>
#include <imgui.h>

namespace game2d {
using namespace std::literals;

void

update_ui_worldspace_text_system(entt::registry& r)
{
  const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);

  const auto& view = r.view<TransformComponent, WorldspaceTextComponent>();
  for (const auto& [e, t, wst_c] : view.each()) {
    const auto eid = static_cast<uint32_t>(e);

    const auto worldspace = position_in_worldspace(r, glm::ivec2(t.position.x, t.position.y));
    ImVec2 pos = { static_cast<float>(worldspace.x), static_cast<float>(worldspace.y) };

    // keeps in screen boundaries?
    pos.x = glm::clamp(static_cast<int>(pos.x), 0, ri.viewport_size_render_at.x);
    pos.y = glm::clamp(static_cast<int>(pos.y), 0, ri.viewport_size_render_at.y);

    ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));

    // ImGui::PushStyleColor(ImGuiCol_, (ImVec4)ImColor::HSV(1 / 7.0f, 0.6f, 0.6f));
    ImGuiWindowFlags flags = 0;
    flags |= ImGuiWindowFlags_NoMove;
    flags |= ImGuiWindowFlags_NoTitleBar;
    flags |= ImGuiWindowFlags_NoBackground;
    flags |= ImGuiWindowFlags_NoResize;
    flags |= ImGuiDockNodeFlags_AutoHideTabBar;
    flags |= ImGuiDockNodeFlags_NoResize;

    ImGui::PushID(eid);
    std::string beginlabel = "WorldspaceText"s;
    ImGui::Begin(beginlabel.c_str(), NULL, flags);
    ImGui::Text("%s", wst_c.text.c_str());
    ImGui::End();
    ImGui::PopID();
    //
  }
}

} // namespace game2d