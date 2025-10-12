#include "pch.hpp"

#include "ui_label_system.hpp"

#include "engine/imgui/ui_imgui_defaults.hpp"
#include "engine/maths/maths.hpp"
#include "engine/renderer/transform.hpp"
#include "engine/sprites/helpers.hpp"
#include "modules/actors/actor_island_cannon/island_cannon_components.hpp"
#include "modules/combat/combat_core/components.hpp"
#include "modules/core/camera/helpers.hpp"
#include "modules/core/fonts/fonts_helpers.hpp"
#include "modules/core/renderer/components.hpp"
#include "modules/core/renderer/helpers.hpp"
#include "modules/core/ui/ui_draw_text_helpers.hpp"
#include "modules/systems/system_island_movement/island_movement_components.hpp"
#include "resources/data.hpp"

namespace game2d {

void
draw_dashed_line(ImDrawList* draw_list, const ImVec2 p0, const ImVec2 p1, int n_dashes, const engine::SRGBColour& scol)
{
  const float width = 2.0f;
  const glm::vec2 raw_dir = glm::vec2{ p1.x, p1.y } - glm::vec2{ p0.x, p0.y };
  const glm::vec2 nrm_dir = engine::normalize_safe(raw_dir);

  // just draw a solid line
  if (n_dashes == 1) {
    const auto col = IM_COL32(scol.r, scol.g, scol.b, 255);
    draw_list->AddLine(p0, p1, col, width);
    return;
  }

  // calculate the length of the canvas
  const float length = glm::length(raw_dir);
  const float total_dash_length = length / n_dashes;
  const float dash_length = total_dash_length * 0.5f; // 0.5 = half gaps, half dash
  const float gap_length = total_dash_length - dash_length;

  // draw each dash
  glm::vec2 start = { p0.x, p0.y };
  for (int i = 0; i < n_dashes; i++) {
    const glm::vec2 end = start + nrm_dir * dash_length;

    // fade from 0 to 1 alpha
    const float percent = (i / float(n_dashes));
    const auto col = IM_COL32(scol.r, scol.g, scol.b, percent * 255);
    draw_list->AddLine({ start.x, start.y }, { end.x, end.y }, col, width);

    start = end + nrm_dir * gap_length;
  }
}

void
update_ui_label_system(entt::registry& r)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
  const auto& ri = SINGLE_RendererInfo::instance;
  const auto screen_size = ImVec2{ (float)ri.viewport_size_render_at.x, (float)ri.viewport_size_render_at.y };
  ImGui::SetNextWindowPos({ 0, 0 }, ImGuiCond_Always, { 0.0f, 0.0f });
  ImGui::SetNextWindowSize(screen_size, ImGuiCond_Always);
  imgui_begin("overlay", ImGuiWindowFlags_NoInputs);
  const auto tl = ImGui::GetWindowPos();
  const auto wh = ImGui::GetWindowSize();
  auto* draw_list = ImGui::GetWindowDrawList();
  auto* font = get_inter_font(r, FontSize::TEXT_SIZE_16);

  for (const auto& [e, t_c, cannon_c] : r.view<const TransformComponent, const IslandCannonComponent>().each()) {
    auto ws_pos = glm::vec2{ t_c.position.x, t_c.position.y };
    const auto ss_pos = worldspace_to_screenspace(r, ws_pos);
    const auto im_ss_pos = ImVec2(ss_pos.x, ss_pos.y);

    const auto hits = cannon_c.hits_to_repair - cannon_c.hits_to_repair_left;

    const bool repaired = cannon_c.hits_to_repair_left == 0;
    auto text = std::format("Needs repair: {}/{}", hits, cannon_c.hits_to_repair);
    if (repaired)
      text = "Repaired. Bullets: X/X.";

    // todo: point p1 off the island
    const auto text_size = font->CalcTextSizeA(font->FontSize, FLT_MAX, -1, text.c_str());
    const auto p0 = im_ss_pos + ImVec2(0, 0);
    const auto p1 = im_ss_pos + ImVec2(16, -16);
    const auto p2 = p1 + ImVec2(text_size.x, 0);

    draw_dashed_line(draw_list, p0, p1, 1, { 1.0f, 1.0f, 1.0f, 1.0f });
    draw_dashed_line(draw_list, p1, p2, 1, { 1.0f, 1.0f, 1.0f, 1.0f });

    auto col = IM_COL32(255, 255, 255, 255);
    if (repaired)
      col = im_greenish;

    const auto text_pos = p1 + ImVec2(0, -text_size.y * 0.5f);
    draw_list->AddText(font, font->FontSize, p1, col, text.c_str());
  }

  ImGui::End();
  ImGui::PopStyleVar(2);
}

} // namespace game2d