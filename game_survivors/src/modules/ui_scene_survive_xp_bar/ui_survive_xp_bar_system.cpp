#include "ui_survive_xp_bar_system.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/imgui/helpers.hpp"
#include "modules/core_renderer/components.hpp"
#include "modules/event_coll_player_xp/event_coll_player_xp_components.hpp"

#include <imgui.h>

namespace game2d {

void
update_ui_survive_xp_bar_system(entt::registry& r)
{
  GET_FIRST_OR_RETURN(SINGLE_XpComponent, r, sxp_e, sxp_c);

  const auto& ri = get_first_component<SINGLE_RendererInfo>(r);

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoDecoration;
  flags |= ImGuiWindowFlags_NoMove;
  flags |= ImGuiWindowFlags_NoDocking;
  flags |= ImGuiWindowFlags_NoSavedSettings;
  flags |= ImGuiWindowFlags_NoFocusOnAppearing;
  flags |= ImGuiWindowFlags_NoInputs;
  flags |= ImGuiWindowFlags_NoBackground;
  // flags |= ImGuiWindowFlags_AlwaysAutoResize;

  const float offset_y = 0;
  const auto bar_height = 16;
  const auto window_size = ImVec2(ri.viewport_size_render_at.x, 100);

  ImGui::SetNextWindowPos({ 0, offset_y }, ImGuiCond_Always, { 0.0f, 0.0f });
  ImGui::SetNextWindowSize(window_size, ImGuiCond_Always);

  // data
  const int current_xp = sxp_c.xp;
  const float max_xp = sxp_c.xp_for_next_level;
  const float percent = current_xp / max_xp;

  // Define colors for the XP bar
  const ImU32 bg = ImColor(50, 50, 50, 255);
  const ImU32 fg = ImColor(100, 200, 100, 255);
  const auto text_col = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

  const float r_adj = 25;
  const float l_adj = 5;
  // imgui_draw_float("debug r_adj", r_adj);
  // imgui_draw_float("debug l_adj", l_adj);

  ImGui::Begin("XPbar", NULL, flags);

  ImVec2 spos = ImGui::GetCursorScreenPos();
  spos.x += l_adj;
  const auto p_min = spos;
  const auto p_max = ImVec2(p_min.x + window_size.x - r_adj, p_min.y + bar_height);
  const auto partial = ImVec2(p_min.x + percent * (window_size.x - r_adj), p_min.y + bar_height);

  // const ImDrawFlags corners = ImDrawFlags_RoundCornersTopLeft | ImDrawFlags_RoundCornersBottomRight;
  const ImDrawFlags corners = ImDrawFlags_RoundCornersAll;
  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  draw_list->AddRectFilled(p_min, p_max, bg, 10.0f, corners);   // background bar
  draw_list->AddRectFilled(p_min, partial, fg, 10.0f, corners); // foreground bar

  {
    ImGuiStyle& style = ImGui::GetStyle();
    const auto ui_wh = ImGui::GetContentRegionAvail();
    const auto ui_tl = ImGui::GetCursorPos();
    const auto ui_center = ImVec2{ ui_tl.x + ui_wh.x * 0.5f, ui_tl.y + ui_wh.y * 0.5f };

    // std::string label = std::format("{} / {}", int(current_xp), max_xp);
    std::string label = std::format("LEVEL {}", sxp_c.level);

    const auto text_size = ImGui::CalcTextSize(label.c_str());
    const float text_size_x = text_size.x + (style.FramePadding.x * 2.0f);
    const float text_size_y = text_size.y + (style.FramePadding.y * 2.0f);

    ImGui::SetCursorPosX(ui_center.x - text_size_x * 0.5f);
    ImGui::SetCursorPosY(ui_center.y - text_size_y * 0.5f);
    ImGui::TextColored(text_col, "%s", label.c_str());
  }

  ImGui::End();
}

} // namespace game2d