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
  GET_FIRST_OR_RETURN(SINGLE_Spawners, r, disk_spawn_data_e, disk_spawn_data_c);

  auto& input_c = get_first_component<SINGLE_InputComponent>(r);

  if (get_key_held(input_c, SDL_SCANCODE_LSHIFT)) {
    // todo: go to next/previous wave
    if (get_key_down(input_c, SDL_SCANCODE_EQUALS))
      timer_c.time_left_cur += 60;
    if (get_key_down(input_c, SDL_SCANCODE_MINUS))
      timer_c.time_left_cur -= 60;
  } else {
    if (get_key_down(input_c, SDL_SCANCODE_EQUALS))
      timer_c.time_left_cur += 20;
    if (get_key_down(input_c, SDL_SCANCODE_MINUS))
      timer_c.time_left_cur -= 20;
  }

  ImGui::Begin("DebugSpawner");

  // const int minutes = static_cast<int>(cooldown_c.time) / 60;
  // const int seconds = static_cast<int>(cooldown_c.time) % 60;
  const int seconds_from_start = timer_c.time_left_max - timer_c.time_left_cur;
  ImGui::Text("Seconds from start: %i", seconds_from_start);

  for (const auto [e, data_c, cooldown_c] : r.view<EnemySpawnData, CooldownComponent>().each()) {

    const std::optional<int> w_idx_opt = get_wave_index_from_time(data_c, seconds_from_start);

    std::string wave_label = std::format("Wave {}", data_c.key);
    ImGui::SeparatorText(wave_label.c_str());

    if (!w_idx_opt.has_value()) {
      ImGui::Text("Wave: no wave data");
      continue;
    }

    const auto wave_key = WaveKey{
      .on_disk_spawns_index = data_c.on_disk_index,
      .on_disk_waves_index = w_idx_opt.value(),
    };
    const auto on_disk_wave = disk_spawn_data_c.spawns[wave_key.on_disk_spawns_index].waves[wave_key.on_disk_waves_index];
    const auto& w = on_disk_wave;

    ImGui::Text("wave_start_seconds %i", min_to_sec(w.span.start));
    ImGui::Text("wave_stop_seconds %i", min_to_sec(w.span.stop));
    ImGui::Text("hp %0.2f", w.hp);
    ImGui::Text("max_allowed %i", w.max);
    ImGui::Text("number_per_spawn %i", w.num_per_spawn);
    ImGui::Text("spawn_cooldown %f", w.spawn_cooldown.value());

    if (w.num_per_wave.has_value())
      ImGui::Text("num_per_wave %i", w.num_per_wave.value());
    else
      ImGui::Text("num_per_wave (infinite)");
  }

  ImGui::End();
}

} // namespace game2d