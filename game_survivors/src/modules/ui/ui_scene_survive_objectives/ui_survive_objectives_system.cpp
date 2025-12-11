#include "pch.hpp"

#include "ui_survive_objectives_system.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/imgui/ui_imgui_defaults.hpp"
#include "modules/actors/actor_lighthouse/lighthouse_components.hpp"
#include "modules/actors/actor_rock/rock_components.hpp"
#include "modules/core/fonts/fonts_helpers.hpp"
#include "modules/core/renderer/components.hpp"
#include "modules/core/ui/ui_common_components.hpp"
#include "modules/systems/system_item_gold/gold_components.hpp"
#include "modules/systems/system_stats/stats_components.hpp"

namespace game2d {

void
update_ui_survive_objectives_system(entt::registry& r)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif

  const auto ui_scaling = get_first_component<SINGLE_UIScaling>(r).scaling;
  const auto font_size = (float)FontSizes::SIZE_16;
  auto* font = get_inter_font(r);
  ImGui::PushFont(font, font_size);

  const auto& ri_c = SINGLE_RendererInfo::instance;

  const auto set_window_pos = ImVec2{ (float)ri_c.viewport_size_render_at.x, 20 }; // top-right
  ImGui::SetNextWindowPos(set_window_pos, ImGuiCond_Always, { 1.0f, 0.0f });       // top right
  // const auto set_window_pos = ImVec2{ 0, 0 };                                // top-left
  // ImGui::SetNextWindowPos(set_window_pos, ImGuiCond_Always, { 0.0f, 0.0f }); // top-left
  imgui_begin("SurviveSceneObjectives", ImGuiWindowFlags_NoInputs);

  ImGui::SeparatorText("Plunder");
  // {
  const auto& stats_c = get_first_component<SINGLE_SurviveStatsComponent>(r);
  auto& gold_c = get_first_component<SINGLE_GoldComponent>(r);

  // hack: this shouldnt be here.
  // this updates the gold amount based on enemies killed,
  // but this shouldnt live in the ui component.
  // gold_c.temp_amount_enemies = glm::max((int)(stats_c.enemies_killed / 50), 0);

  const auto gold = gold_c.temp_amount_pickup + gold_c.temp_amount_enemies;
  ImGui::Text("%s", std::format("+{}g", gold).c_str());
  // }

  ImGui::NewLine();
  ImGui::SeparatorText("Sub-Objectives");

  const int island_count = (int)r.view<const DebugContoursComponent>().size();
  const int lighthouse_count = (int)r.view<const LighthouseComponent>().size();
  ImGui::Text("Islands Captured: %i/%i", lighthouse_count, island_count);

  if (island_count == lighthouse_count) {
    ImGui::SameLine();
    ImGui::Text(" (Complete)");
  }

  ImGui::PopFont();

  ImGui::End();
}

} // namespace game2d