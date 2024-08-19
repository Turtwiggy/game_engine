#include "system.hpp"

#include "colour/colour.hpp"
#include "entt/helpers.hpp"
#include "maths/maths.hpp"
#include "modules/camera/components.hpp"
#include "modules/camera/helpers.hpp"
#include "modules/system_distance_check/components.hpp"
#include "modules/ui_overworld_ship_label/components.hpp"
#include "renderer/transform.hpp"

#include "imgui.h"

namespace game2d {
using namespace std::literals;

void
draw_dashed_line(ImDrawList* draw_list, const ImVec2 p0, const ImVec2 p1, int n_dashes, const engine::SRGBColour& scol)
{
  const float width = 2.0f;
  const ImVec2 c_p0 = ImGui::GetCursorScreenPos(); // ImDrawList API uses screen coordinates!

  const glm::vec2 raw_dir = glm::vec2{ p1.x, p1.y } - glm::vec2{ p0.x, p0.y };
  const glm::vec2 nrm_dir = engine::normalize_safe(raw_dir);

  // just draw a solid line
  if (n_dashes == 1) {
    const auto col = IM_COL32(scol.r, scol.g, scol.b, 255);
    draw_list->AddLine(ImVec2(c_p0.x + p0.x, c_p0.y + p0.y), ImVec2(c_p0.x + p1.x, c_p0.y + p1.y), col, width);
    return;
  }

  // calculate the length of the canvas
  float length = glm::length(raw_dir);
  float total_dash_length = length / n_dashes;
  float dash_length = total_dash_length * 0.5f; // half gaps, half dash
  float gap_length = total_dash_length - dash_length;

  // draw each dash
  glm::vec2 start = { p0.x, p0.y };
  for (size_t i = 0; i < n_dashes; i++) {
    const glm::vec2 end = start + nrm_dir * dash_length;

    // fade from 0 to 1 alpha
    const float percent = (i / float(n_dashes));
    const auto col = IM_COL32(scol.r, scol.g, scol.b, percent * 255);
    draw_list->AddLine(ImVec2(c_p0.x + start.x, c_p0.y + start.y), ImVec2(c_p0.x + end.x, c_p0.y + end.y), col, width);

    start = end + nrm_dir * gap_length;
  }
}

void
update_ui_overworld_shiplabel_system(entt::registry& r)
{
  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoMove;
  flags |= ImGuiWindowFlags_NoTitleBar;
  flags |= ImGuiWindowFlags_NoBackground;
  flags |= ImGuiDockNodeFlags_AutoHideTabBar;
  flags |= ImGuiDockNodeFlags_NoResize;

  const auto& view = r.view<const SpaceLabelComponent, const TransformComponent>();
  for (const auto& [e, label_c, t_c] : view.each()) {
    const auto eid = static_cast<uint32_t>(e);

    // configs
    const auto text_size = ImGui::CalcTextSize(label_c.text.c_str());
    const ImVec2 window_size{ 50 + text_size.x, 50 };
    const ImVec2 text_pos = { window_size.x - text_size.x, 0 };
    float extra_underline_separation = 4;

    // position in worldspace
    const auto t_pos = glm::ivec2(t_c.position.x, t_c.position.y);
    const auto worldspace = position_in_worldspace(r, t_pos);
    const ImVec2 pos = { static_cast<float>(worldspace.x), static_cast<float>(worldspace.y) };

    // set window position
    const ImVec2 final_offset = { pos.x, pos.y - window_size.y };
    ImGui::SetNextWindowPos(final_offset);
    ImGui::SetNextWindowSizeConstraints(window_size, window_size);
    ImGui::SetNextWindowSize(window_size);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0)); // Disable padding
    const std::string beginlabel = "overworld-ui-labels##"s + std::to_string(eid);
    ImGui::Begin(beginlabel.c_str(), NULL, flags);
    ImGui::PopStyleVar();
    ImGui::PushID(eid);

    const ImVec2 text_pos_underline_l = { text_pos.x, text_pos.y + text_size.y + extra_underline_separation };
    const ImVec2 text_pos_underline_r = { text_pos.x + text_size.x, text_pos.y + text_size.y + extra_underline_separation };

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    draw_dashed_line(draw_list, { 0, window_size.y }, text_pos_underline_l, 10, label_c.ui_colour);
    draw_dashed_line(draw_list, text_pos_underline_l, text_pos_underline_r, 1, label_c.ui_colour);

    ImGui::SetCursorPos(text_pos);
    ImGui::Text("%s", label_c.text.c_str());

    ImGui::PopID();
    ImGui::End();
  }
}

} // namespace game2d