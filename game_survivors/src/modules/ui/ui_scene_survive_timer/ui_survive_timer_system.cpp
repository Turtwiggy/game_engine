#include "pch.hpp"

#include "ui_survive_timer_system.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/imgui/ui_imgui_defaults.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/actors/actor_snake/snake_components.hpp"
#include "modules/core/fonts/fonts_helpers.hpp"
#include "modules/core/renderer/components.hpp"
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
  const auto& ri = SINGLE_RendererInfo::instance;
  const glm::vec2 tr = ri.viewport_size_render_at;

  const auto players_view = r.view<PlayerComponent>();
  if (players_view.size() == 0)
    return; // no players, dont count down timer

  for (const auto& [e, timer_c] : r.view<const SurviveTimerComponent>().each()) {

    // countdown
    const int seconds = static_cast<int>(timer_c.time_left_cur) % 60;
    const int minutes = static_cast<int>(timer_c.time_left_cur) / 60;

    // countup minutes
    // const int cu_minutes = 19 - minutes;
    // const int cu_seconds = 59 - seconds;

    const auto display = std::format("{:02}:{:02}", minutes, seconds);
    const auto len = ImGui::CalcTextSize(display.c_str());
    const auto padding = ImGui::GetStyle().WindowPadding;
    const int distance_from_top_of_screen = 0;

    const auto pos = ImVec2{ tr.x, distance_from_top_of_screen };
    ImGui::SetNextWindowPos(pos, ImGuiCond_Always, { 1.0f, 0.0f });
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(6.0f, 5.0f));

    const auto eid = static_cast<uint32_t>(e);
    ImGui::PushID(eid);
    imgui_begin("timer");

    const auto ui_wh = ImGui::GetContentRegionAvail();
    const auto ui_tl = ImGui::GetCursorPos();

    {
      const auto header_font_scale = get_first_component<SINGLE_UIScaling>(r).scaling;
      const auto header_font_enum = header_font_scale == 1.0f ? FontSize::GAME_TIMER : FontSize::GAME_TIMER_SCALED;
      auto* header_font = get_inter_font(r, header_font_enum);
      ImGui::PushFont(header_font); // Use the larger font (index 1)

      ImGui::Text("%s", display.c_str());

      ImGui::PopFont();
    }
    {
      const auto font_scale = get_first_component<SINGLE_UIScaling>(r).scaling;
      const auto font_enum = font_scale == 1.0f ? FontSize::TEXT_SIZE_16 : FontSize::TEXT_SIZE_16_SCALED;
      auto* font = get_inter_font(r, font_enum);
      ImGui::PushFont(font); // Use the larger font (index 1)

      const auto my_orange_col = hex_to_srgb("#DF9755");
      const auto im_orange_col = convert_my_to_im_vec(my_orange_col);
      const float padding_x = 10.0;

      if (r.view<BossComponent>().size() > 0) {
        const std::string str = "Defeat Sea-Snake!";
        const auto str_size = ImGui::CalcTextSize(str.c_str());
        ImGui::Text("");
        ImGui::SameLine(ui_wh.x - str_size.x);
        ImGui::TextColored(im_orange_col, "%s", str.c_str());
      } else {
        const std::string str = "Survive!";
        const auto str_size = ImGui::CalcTextSize(str.c_str());
        ImGui::Text("");
        ImGui::SameLine(ui_wh.x - str_size.x);
        ImGui::TextColored(im_orange_col, "%s", str.c_str());
      }

      ImGui::PopFont();
    }

    ImGui::End();
    ImGui::PopID();
    ImGui::PopStyleVar();

    break; // only one timer
  };
}

} // namespace game2d