#include "pch.hpp"

#include "ui_survive_timer_system.hpp"

#include "engine/entt/helpers.hpp"
#include "modules/actor_snake/snake_components.hpp"
#include "modules/core_fonts/fonts_helpers.hpp"
#include "modules/core_renderer/components.hpp"
#include "modules/ui_common/ui_common_components.hpp"
#include "ui_survive_timer_components.hpp"

namespace game2d {

void
update_ui_survive_timer_system(entt::registry& r)
{
  const auto& ri = get_first_component<SINGLE_RendererInfo>(r);
  const glm::vec2 tr = ri.viewport_size_render_at;

  const auto font_scale = get_first_component<SINGLE_UIData>(r).scaling;
  const auto font_enum = font_scale == 1.0f ? FontSize::TEXT_LARGE : FontSize::TEXT_LARGE_SCALED;
  auto* font = get_fingerpaint_font(r, font_enum);
  ImGui::PushFont(font); // Use the larger font (index 1)

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
    const auto space = 8;
    const int distance_from_top_of_screen = 20;

    const auto pos = ImVec2{ tr.x, distance_from_top_of_screen };
    ImGui::SetNextWindowPos(pos, ImGuiCond_Always, { 1.0f, 0.0f });

    ImGuiWindowFlags flags = 0;
    flags |= ImGuiWindowFlags_NoDecoration;
    flags |= ImGuiWindowFlags_NoMove;
    flags |= ImGuiWindowFlags_NoBackground;
    flags |= ImGuiWindowFlags_NoDocking;
    flags |= ImGuiWindowFlags_NoSavedSettings;
    flags |= ImGuiWindowFlags_NoFocusOnAppearing;
    flags |= ImGuiWindowFlags_NoInputs;
    flags |= ImGuiWindowFlags_AlwaysAutoResize;

    const auto eid = static_cast<uint32_t>(e);
    ImGui::PushID(eid);
    ImGui::Begin("Timer", NULL, flags);

    ImGui::Text("%s", display.c_str());

    if (r.view<BossComponent>().size() > 0)
      ImGui::Text("Kill Steve");

    ImGui::End();
    ImGui::PopID();

    break; // only one timer
  };

  ImGui::PopFont(); // Restore default font
}

} // namespace game2d