#include "pch.hpp"

#include "ui_display_build_number_system.hpp"

#include "engine/imgui/ui_imgui_defaults.hpp"
#include "modules/core/renderer/components.hpp"
#include "resources/data.hpp"

namespace game2d {

void
update_ui_display_build_number_system(entt::registry& r)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif
  const auto& ri = SINGLE_RendererInfo::instance;

  const auto viewport_tl = ImVec2((float)ri.viewport_pos.x, (float)ri.viewport_pos.y);
  const auto viewport_wh = ImVec2(ri.viewport_size_render_at.x, ri.viewport_size_render_at.y - 100);

  auto str = "Version 0.38";
  auto str1 = std::format("Suggestions, feedback, wizards? {} ", discord_link);
  auto line_size_x = glm::max(ImGui::CalcTextSize(str).x, ImGui::CalcTextSize(str1.c_str()).x);

  auto padding = ImVec2(10, -10);
  const auto pos = ImVec2(0 + padding.x, viewport_tl.y + viewport_wh.y + padding.y);
  ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.0f, 1.0f));

  imgui_begin("build_number");

  ImGui::Text("%s", str1.c_str());
  ImGui::SameLine();
  if (ImGui::Button("Copy")) {
    ImGui::LogToClipboard();
    ImGui::LogText("%s", discord_link.c_str());
    ImGui::LogFinish();
  }

  ImGui::Text("%s", str); // todo: get from version control innit

  ImGui::End();
}

} // namespace game2d