#include "modules/ui_worldspace_text/system.hpp"

#include "components.hpp"
#include "entt/helpers.hpp"
#include "modules/camera/helpers.hpp"
#include "modules/renderer/components.hpp"
#include "renderer/transform.hpp"

#include <glm/glm.hpp>
#include <imgui.h>

namespace game2d {
using namespace std::literals;

const auto split_paragrah = [](const std::string& str, const int& len) -> std::vector<std::string> {
  std::vector<std::string> results;

  int start = 0;
  while (start < str.size()) {
    int end = start + len;

    // Get the nearest ' ' char to the end of the sentence
    if (end < str.size() && str[end] != ' ') {
      const int space_pos = str.rfind(' ', end);
      if (space_pos != std::string::npos && space_pos > start)
        end = space_pos;
    }

    // add the sentence so far
    results.push_back(str.substr(start, end - start));

    // onwards..!
    start = end;
    while (start < str.size() && str[start] == ' ')
      ++start; // Skip spaces
  }

  return results;
};

void
update_ui_worldspace_text_system(entt::registry& r)
{
  const auto& ri = get_first_component<SINGLETON_RendererInfo>(r);

  const auto& view = r.view<TransformComponent, WorldspaceTextComponent>();
  for (const auto& [e, t, wst_c] : view.each()) {
    const auto eid = static_cast<uint32_t>(e);

    const auto t_pos = glm::ivec2(t.position.x, t.position.y);
    const auto worldspace = position_in_worldspace(r, t_pos);
    ImVec2 pos = { static_cast<float>(worldspace.x), static_cast<float>(worldspace.y) };
    pos.x += wst_c.offset.x;
    pos.y += wst_c.offset.y;

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

    std::string beginlabel = "WorldspaceText##"s + std::to_string(eid);
    ImGui::Begin(beginlabel.c_str(), NULL, flags);
    ImGui::PushID(eid);

    // if (wst_c.font_scale != 1.0f)
    //   ImGui::SetWindowFontScale(wst_c.font_scale);

    const auto shortened = split_paragrah(wst_c.text, 20);
    for (const auto& line : shortened)
      ImGui::Text("%s", line.c_str());

    // if (wst_c.font_scale != 1.0f)
    //   ImGui::SetWindowFontScale(1.0f);

    ImGui::PopID();
    ImGui::End();
  }
}

} // namespace game2d