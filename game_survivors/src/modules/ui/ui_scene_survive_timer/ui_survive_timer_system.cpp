#include "pch.hpp"

#include "ui_survive_timer_system.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/imgui/ui_imgui_defaults.hpp"
#include "engine/sprites/helpers.hpp"
#include "modules/actors/actor_boat/boat_components.hpp"
#include "modules/core/fonts/fonts_helpers.hpp"
#include "modules/core/renderer/components.hpp"
#include "modules/core/renderer/helpers.hpp"
#include "modules/core/ui/ui_common_components.hpp"
#include "modules/ui/ui_colours/ui_colours_helpers.hpp"
#include "ui_survive_timer_components.hpp"

namespace game2d {

void
update_ui_survive_timer_system(entt::registry& r)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif
  const auto& ri_c = SINGLE_RendererInfo::instance;
  const glm::vec2 tr = ri_c.viewport_size_render_at;

  const auto players_view = r.view<PlayerBoatComponent>();
  if (players_view.empty())
    return; // no players, dont count down timer

  const auto view = r.view<const SurviveTimerComponent>();
  const auto survive_e = view.front();
  const auto& timer_c = r.get<const SurviveTimerComponent>(survive_e);

  // countdown
  const int seconds = static_cast<int>(timer_c.time_left_cur) % 60;
  const int minutes = static_cast<int>(timer_c.time_left_cur) / 60;
  // countup
  // const int cu_minutes = 19 - minutes;
  // const int cu_seconds = 59 - seconds;

  const auto pos = ImVec2{ 0.33f * ri_c.viewport_size_render_at.x, 0 };
  const auto size = ImVec2((float)0.334f * ri_c.viewport_size_render_at.x, 80);
  ImGui::SetNextWindowPos(pos, ImGuiCond_Always, { 0.0f, 0.0f });
  ImGui::SetNextWindowSize(size, ImGuiCond_Always);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(6.0f, 5.0f));

  imgui_begin("timer");
  const ImVec2 ui_tl = ImGui::GetWindowPos();
  const ImVec2 ui_wh = ImGui::GetWindowSize();
  auto* draw_list = ImGui::GetWindowDrawList();

  // display a bar.
  const auto header_font_scale = get_first_component<SINGLE_UIScaling>(r).scaling;
  const auto header_font_enum = header_font_scale == 1.0f ? FontSize::TEXT_LARGE : FontSize::TEXT_LARGE_SCALED;
  const auto my_fg_col = hex_to_srgb("#508FC8");
  const auto my_bg_col = hex_to_srgb("#293448", 100);
  const auto my_icon_col = hex_to_srgb("#FF0000", 200);
  const auto im_fg_col = convert_my_to_im(my_fg_col);
  const auto im_bg_col = convert_my_to_im(my_bg_col);
  const auto im_icon_col = convert_my_to_im(my_icon_col);
  const float bar_height = 12.0f * header_font_scale;
  const float bar_tl_y = 8;
  const float rounding = 8.0f;

  // display time.
  const auto display = std::format("{:02}:{:02}", minutes, seconds);
  const auto padding = ImGui::GetStyle().WindowPadding;
  auto* header_font = get_inter_font(r, header_font_enum);
  ImGui::PushFont(header_font); // Use the larger font (index 1)
  const auto len = ImGui::CalcTextSize(display.c_str());
  ImGui::SetCursorPos({ 0.5f * (ui_wh.x - len.x), 0.5f * (ui_wh.y - len.y) });
  ImGui::Text("%s", display.c_str());
  ImGui::PopFont();

  // draw timer bar bg
  const auto bar_tl = ImVec2(ui_tl.x, bar_tl_y);
  const auto bar_br = ImVec2(ui_tl.x + ui_wh.x, bar_tl_y + bar_height);
  draw_list->AddRectFilled(bar_tl, bar_br, im_bg_col, rounding, ImDrawFlags_RoundCornersAll);

  // draw timer bar fg
  const float percent = 1.0f - (timer_c.time_left_cur / timer_c.time_left_max);
  const auto fg_bar_tl = ImVec2(ui_tl.x, bar_tl_y);
  const auto fg_bar_br = ImVec2(ui_tl.x + percent * ui_wh.x, bar_tl_y + bar_height);
  draw_list->AddRectFilled(fg_bar_tl, fg_bar_br, im_fg_col, rounding, ImDrawFlags_RoundCornersAll);

  // draw a skull icon halfway on the bar
  {
    const auto icon = "SKULL_AND_BONES";
    const auto icon_size_half = 10.0f * header_font_scale;
    const auto icon_padding = 0;
    const auto tex_id = search_for_texture_id_by_texture_path(ri_c, "monochrome")->id;
    const auto im_id = (ImTextureID)(void*)(intptr_t)tex_id;
    const auto bar_center = ImVec2{ 0.5f * (bar_tl.x + bar_br.x), 0.5f * (bar_tl.y + bar_br.y) };
    const auto icon_tl = ImVec2(bar_center.x - icon_size_half, bar_center.y - icon_size_half);
    const auto icon_br = ImVec2(bar_center.x + icon_size_half, bar_center.y + icon_size_half);
    const auto [icon_uv_tl, icon_uv_br] = convert_sprite_to_uv(r, icon);
    const auto icon_p_tl = ImVec2{ icon_tl.x + icon_padding, icon_tl.y + icon_padding };
    const auto icon_p_br = ImVec2{ icon_br.x - icon_padding, icon_br.y - icon_padding };
    // draw_list->AddRectFilled(icon_p_tl, icon_p_br, IM_COL32(255, 0, 0, 255), rounding, ImDrawFlags_RoundCornersBottom);
    draw_list->AddImage(im_id, icon_p_tl, icon_p_br, icon_uv_tl, icon_uv_br, im_icon_col);
  }

  // if (r.view<BossComponent>().size() > 0) {

  ImGui::End();
  ImGui::PopStyleVar();
}

} // namespace game2d