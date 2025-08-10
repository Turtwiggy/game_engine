#include "pch.hpp"

#include "ui_survive_objectives_system.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/imgui/ui_imgui_defaults.hpp"
#include "modules/actors/actor_lighthouse/lighthouse_components.hpp"
#include "modules/actors/actor_rock/rock_components.hpp"
#include "modules/core/fonts/fonts_helpers.hpp"
#include "modules/core/ui/ui_common_components.hpp"

namespace game2d {

void
update_ui_survive_objectives_system(entt::registry& r)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif

  const auto ui_scaling = get_first_component<SINGLE_UIScaling>(r).scaling;
  const auto font_enum_16 = ui_scaling == 1.0f ? FontSize::TEXT_SIZE_16 : FontSize::TEXT_SIZE_16_SCALED;
  auto* font_16 = get_inter_font(r, font_enum_16);

  const auto set_window_pos = ImVec2{ 0, 0 }; // top-left
  ImGui::SetNextWindowPos(set_window_pos, ImGuiCond_Always, { 0.0f, 0.0f });

  imgui_begin("SurviveSceneObjectives", ImGuiWindowFlags_NoInputs);

  ImGui::PushFont(font_16);
  ImGui::Text("Sub Objectives");

  const int island_count = (int)r.view<DebugContoursComponent>().size();
  const int lighthouse_count = (int)r.view<LighthouseComponent>().size();
  ImGui::Text("%s", std::format("Islands Captured: {}/{}", lighthouse_count, island_count).c_str());

  if (island_count == lighthouse_count) {
    ImGui::SameLine();
    ImGui::Text(" (Complete)");
  }

  ImGui::PopFont();

  ImGui::End();
}

} // namespace game2d