#include "pch.hpp"

#include "ui_survive_xp_bar_system.hpp"

#include "engine/entt/helpers.hpp"
#include "modules/core_fonts/fonts_helpers.hpp"
#include "modules/core_renderer/components.hpp"
#include "modules/event_coll_player_xp/event_coll_player_xp_components.hpp"
#include "modules/ui_colours/ui_colours_helpers.hpp"
#include "modules/ui_common/ui_common_components.hpp"

namespace game2d {

void
update_ui_survive_xp_bar_system(entt::registry& r)
{
  GET_FIRST_OR_RETURN(SINGLE_XpComponent, r, sxp_e, sxp_c);
  const auto& ri = get_first_component<SINGLE_RendererInfo>(r);

  ImGuiWindowFlags flags = 0;
  flags |= ImGuiWindowFlags_NoDecoration;
  flags |= ImGuiWindowFlags_NoMove;
  flags |= ImGuiWindowFlags_NoInputs;
  flags |= ImGuiWindowFlags_NoBackground;
  flags |= ImGuiWindowFlags_NoDocking;

  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 0.0f, 0.0f });
  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0.0f, 0.0f });
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);

  const auto font_scale = get_first_component<SINGLE_UIData>(r).scaling;
  const auto font_enum = font_scale == 1.0f ? FontSize::TEXT_LARGE : FontSize::TEXT_LARGE_SCALED;
  auto* font = get_fingerpaint_font(r, font_enum);

  const float bar_height = 8.0f * font_scale;
  const float font_height = (float)font_enum;
  const float padding_y = 4;      // distance from bottom of screen
  const float bar_padding_x = 15; // each side of the screen
  const float total_height = bar_height + font_height;

  const auto window_size = ImVec2((float)ri.viewport_size_render_at.x - 2.0f * bar_padding_x, total_height);
  ImGui::SetNextWindowPos(
    { bar_padding_x, ri.viewport_size_render_at.y - total_height - padding_y }, ImGuiCond_Always, { 0.0f, 0.0f });
  ImGui::SetNextWindowSize(window_size, ImGuiCond_Always);

  // data
  const int current_xp = sxp_c.xp;
  const float max_xp = sxp_c.xp_for_next_level;
  const float percent = current_xp / max_xp;

  // Define colors for the XP bar
  const auto my_fg_col = hex_to_srgb("#508FC8");
  const auto my_bg_col = hex_to_srgb("#293448");
  const auto im_fg_col = convert_my_to_im(my_fg_col);
  const auto im_bg_col = convert_my_to_im(my_bg_col);
  const auto text_col = IM_COL32(255, 255, 255, 255);

  ImGui::Begin("XpBar", NULL, flags);

  const auto ui_tl = ImGui::GetWindowPos();
  const auto ui_wh = ImGui::GetWindowSize();
  auto* draw_list = ImGui::GetWindowDrawList();

  // Draw some text for the level.
  const auto label = std::format("Lv {}", sxp_c.level);
  draw_list->AddText(font, (float)font_enum, ui_tl, text_col, label.c_str());
  // draw_list->AddRectFilled(ui_tl, ui_tl + ui_wh, IM_COL32(255, 255, 255, 255));

  // start the xp bar below the text
  const float new_tl_y = ui_tl.y + font_height;
  const float rem_h = ui_wh.y - font_height;

  // Draw the xp bar bg.
  const auto bar_tl = ImVec2(ui_tl.x, new_tl_y);
  const auto bar_br = ImVec2(ui_tl.x + ui_wh.x, new_tl_y + rem_h);
  draw_list->AddRectFilled(bar_tl, bar_br, im_bg_col);

  // Draw the xp bar fg.
  const auto bar_fg_tl = ImVec2(ui_tl.x, new_tl_y);
  const auto bar_fg_br = ImVec2(ui_tl.x + percent * ui_wh.x, new_tl_y + rem_h);
  draw_list->AddRectFilled(bar_fg_tl, bar_fg_br, im_fg_col);

  // Draw some blocks at 1/4, 2/4, 3/4 to split up the bar..
  // todo

  ImGui::End();
  ImGui::PopStyleVar(4);
}

} // namespace game2d