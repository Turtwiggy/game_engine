#include "ui_debug_spawner_system.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/events/components.hpp"
#include "engine/events/helpers/keyboard.hpp"
#include "modules/system_cooldown/components.hpp"
#include "modules/system_spawner/spawner_components.hpp"
#include "modules/system_spawner/spawner_helpers.hpp"
#include "modules/ui_debug_menubar/ui_debug_menubar_components.hpp"
#include "modules/ui_debug_menubar/ui_debug_menubar_helpers.hpp"
#include "modules/ui_scene_survive_timer/ui_survive_timer_components.hpp"

#include <imgui.h>

namespace game2d {

void
update_ui_debug_spawner_system(entt::registry& r)
{
  auto& menu_c = get_first_component<SINGLE_DebugMenuBar>(r);
  auto ui_state = gesert_menubar_state(menu_c, "Spawners");
  if (!ui_state.enabled)
    return;

  GET_FIRST_OR_RETURN(SurviveTimerComponent, r, timer_e, timer_c);
  auto& cooldown_c = r.get<CooldownComponent>(timer_e);
  auto& input_c = get_first_component<SINGLE_InputComponent>(r);

  if (get_key_held(input_c, SDL_SCANCODE_LSHIFT)) {
    // todo: go to next/previous wave
    if (get_key_down(input_c, SDL_SCANCODE_EQUALS))
      cooldown_c.time += 60;
    if (get_key_down(input_c, SDL_SCANCODE_MINUS))
      cooldown_c.time -= 60;
  } else {
    if (get_key_down(input_c, SDL_SCANCODE_EQUALS))
      cooldown_c.time += 20;
    if (get_key_down(input_c, SDL_SCANCODE_MINUS))
      cooldown_c.time -= 20;
  }

  ImGui::Begin("DebugSpawner");

  // const int minutes = static_cast<int>(cooldown_c.time) / 60;
  // const int seconds = static_cast<int>(cooldown_c.time) % 60;
  const int seconds_from_start = cooldown_c.time_max - (int)cooldown_c.time;
  ImGui::Text("Seconds from start: %i", seconds_from_start);

  const auto enemy_to_amount = get_live_enemies_map(r);

  const auto& view = r.view<EnemySpawnData, CooldownComponent>();
  for (const auto [e, data_c, cooldown_c] : view.each()) {
    const auto w_opt = get_wave_from_time(data_c, seconds_from_start);

    std::string wave_label = std::format("Wave {}", data_c.enemy_key);
    ImGui::SeparatorText(wave_label.c_str());

    if (enemy_to_amount.contains(data_c.enemy_key))
      ImGui::Text("Alive: %i", enemy_to_amount.at(data_c.enemy_key));
    else
      ImGui::Text("Alive: 0");

    if (!w_opt.has_value()) {
      ImGui::Text("Wave: no wave data");
      continue;
    }
    auto& w = w_opt.value();

    ImGui::Text("wave_start_seconds %i", w.wave_start_seconds);
    ImGui::Text("wave_end_seconds %i", w.wave_end_seconds);
    ImGui::Text("hp %0.2f", w.hp);
    ImGui::Text("max_allowed %i", w.max_allowed);
    ImGui::Text("number_per_spawn %i", w.number_per_spawn);
    ImGui::Text("spawn_cooldown %i", w.spawn_cooldown);
  }

  ImGui::End();
}

} // namespace game2d