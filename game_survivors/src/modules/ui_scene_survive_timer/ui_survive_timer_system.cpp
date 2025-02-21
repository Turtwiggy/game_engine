#include "ui_survive_timer_system.hpp"

#include "engine/entt/helpers.hpp"
#include "modules/core_renderer/components.hpp"
#include "modules/system_cooldown/components.hpp"
#include "ui_survive_timer_components.hpp"

#include <imgui.h>

namespace game2d {

void
update_ui_survive_timer_system(entt::registry& r)
{
  const auto& ri = get_first_component<SINGLE_RendererInfo>(r);
  const glm::vec2 tr = ri.viewport_size_render_at;
  const int distance_from_top_of_screen = 20;

  // Push larger font
  ImGuiIO& io = ImGui::GetIO();
  ImGui::PushFont(io.Fonts->Fonts[1]); // Use the larger font (index 1)

  for (const auto& [e, timer_c, cooldown_c] : r.view<const SurviveTimerComponent, const CooldownComponent>().each()) {

    // countdown
    const int seconds = static_cast<int>(cooldown_c.time) % 60;
    const int minutes = static_cast<int>(cooldown_c.time) / 60;

    // countup minutes
    const int cu_minutes = 19 - minutes;
    const int cu_seconds = 59 - seconds;

    const auto display = std::format("{:02}:{:02}", cu_minutes, cu_seconds);
    const auto len = ImGui::CalcTextSize(display.c_str());
    const auto padding = ImGui::GetStyle().WindowPadding;
    const auto space = 8;

    const auto pos = ImVec2{ tr.x - len.x - padding.x - space, distance_from_top_of_screen };
    ImGui::SetNextWindowPos(pos, ImGuiCond_Always, { 0.0f, 0.0f });

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

    ImGui::End();
    ImGui::PopID();

    break; // only one timer
  };

  ImGui::PopFont(); // Restore default font
}

} // namespace game2d