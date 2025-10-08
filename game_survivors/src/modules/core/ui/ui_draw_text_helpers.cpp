#include "pch.hpp"

#include "ui_draw_text_helpers.hpp"

#include "engine/imgui/ui_imgui_defaults.hpp"
#include "modules/core/camera/helpers.hpp"
#include "modules/core/renderer/components.hpp"

namespace game2d {

void
draw_text(entt::registry& r, const WorldspaceText& text)
{
  const auto& ri = SINGLE_RendererInfo::instance;

  const auto screen_size = ImVec2{ (float)ri.viewport_size_render_at.x, (float)ri.viewport_size_render_at.y };
  ImGui::SetNextWindowPos({ 0, 0 }, ImGuiCond_Always, { 0.0f, 0.0f });
  ImGui::SetNextWindowSize(screen_size, ImGuiCond_Always);
  imgui_begin("overlay", ImGuiWindowFlags_NoInputs);

  const auto wsp = glm::vec2(text.worldspace_position.x, text.worldspace_position.y);
  const auto wsp_adj = glm::vec2{ wsp.x, wsp.y };
  const auto ss_pos = worldspace_to_screenspace(r, wsp_adj);
  ImGui::SetCursorScreenPos({ ss_pos.x, ss_pos.y });

  ImGui::Text("%s", text.text.c_str());

  ImGui::End();
}

} // namespace game2d