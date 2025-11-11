#include "pch.hpp"

#include "ui_debug_spawner_system.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/events/components.hpp"
#include "engine/events/helpers/keyboard.hpp"
#include "modules/systems/system_cooldown/components.hpp"
#include "modules/systems/system_spawner/spawner_components.hpp"
#include "modules/systems/system_spawner/spawner_helpers.hpp"
#include "modules/ui/ui_debug_menubar/ui_debug_menubar_components.hpp"
#include "modules/ui/ui_debug_menubar/ui_debug_menubar_helpers.hpp"
#include "modules/ui/ui_scene_survive_timer/ui_survive_timer_components.hpp"

namespace game2d {

void
update_ui_wave_spawner(entt::registry& r, int seconds_from_start)
{
  GET_FIRST_OR_RETURN(SpawnerLiveData, r, live_spawn_data_e, live_spawn_data_c);

  ImGui::SeparatorText("WaveSpawner");

  for (const auto& [spawner_e, cooldown_c, wave] : r.view<const CooldownComponent, const EnemyWavesData>().each()) {

    // Filter wave by time.
    const bool in_lower_bound = seconds_from_start >= min_to_sec(wave.time.start);
    const bool in_upper_bound = seconds_from_start < min_to_sec(wave.time.stop);
    if (!in_lower_bound || !in_upper_bound) {
      ImGui::Text("wave %0.1f %0.1f", wave.time.start, wave.time.stop);
      continue;
    }

    ImGui::Text("(active) wave: %.1f %0.1f", wave.time.start, wave.time.stop);
    ImGui::Text("(active) enemies: %zu", wave.enemies.size());
    ImGui::Text("cooldown: %f", cooldown_c.time);

    for (int i = 0; (int)i < wave.enemies.size(); i++) {

      const WaveSpawnerWaveKey wave_key{
        .idx_in_wave_spawner = wave.on_disk_index,
        .idx_in_wave_spawner_enemy = i,
      };

      const auto has_wave_data = live_spawn_data_c.wavespawner_data.contains(wave_key);
      if (!has_wave_data)
        live_spawn_data_c.wavespawner_data[wave_key] = {};

      const int spawned = live_spawn_data_c.wavespawner_data[wave_key].spawned;
      ImGui::Text("Enemy %s spawned: %i", wave.enemies[i].key.c_str(), spawned);
    }

    //
  }
};

void
update_ui_enemy_spawner(entt::registry& r, int seconds_from_start)
{
  GET_FIRST_OR_RETURN(SINGLE_OnDiskSpawners, r, disk_spawn_data_e, disk_spawn_data_c);

  ImGui::SeparatorText("EnemySpawner");

  for (const auto [e, data_c, cooldown_c] : r.view<const EnemySpawnsData, const CooldownComponent>().each()) {

    const std::optional<int> w_idx_opt = get_wave_index_from_time(data_c, seconds_from_start);

    std::string wave_label = std::format("Wave {}", data_c.key);
    ImGui::SeparatorText(wave_label.c_str());

    if (!w_idx_opt.has_value()) {
      ImGui::Text("Wave: no wave data");
      continue;
    }

    const auto wave_key = EnemySpawnerWaveKey{
      .idx_in_enemy_spawner = data_c.on_disk_index,
      .idx_in_enemy_spawner_waves = w_idx_opt.value(),
    };

    const auto on_disk_wave =
      disk_spawn_data_c.enemy_spawner[wave_key.idx_in_enemy_spawner].waves[wave_key.idx_in_enemy_spawner_waves];
    const auto& w = on_disk_wave;

    ImGui::Text("wave_start_seconds %i", min_to_sec(w.span.start));
    ImGui::Text("wave_stop_seconds %i", min_to_sec(w.span.stop));
    ImGui::Text("hp %0.2f", w.data.hp);
    ImGui::Text("max_allowed %i", w.data.max);
    ImGui::Text("number_per_spawn %i", w.data.num_per_spawn);
    // ImGui::Text("spawn_cooldown %f", w.data.spawn_cooldown.value());

    if (w.data.num_per_wave.has_value())
      ImGui::Text("num_per_wave %i", w.data.num_per_wave.value());
    else
      ImGui::Text("num_per_wave (infinite)");
  }
}

void
update_ui_random_spawner(entt::registry& r, int seconds_from_start)
{
  GET_FIRST_OR_RETURN(SpawnerLiveData, r, live_spawn_data_e, live_spawn_data_c);

  ImGui::SeparatorText("RandomSpawner");

  for (const auto& [spawner_e, cooldown_c, wave] : r.view<const CooldownComponent, const EnemyRandomData>().each()) {

    // ImGui::Text("chosen_enemy: %s", wave.keys[wave.chosen_key_idx].c_str());

    // Filter wave by time.
    const bool in_lower_bound = seconds_from_start >= min_to_sec(wave.time.start);
    const bool in_upper_bound = seconds_from_start < min_to_sec(wave.time.stop);
    if (!in_lower_bound || !in_upper_bound) {
      ImGui::Text("wave %0.1f %0.1f", wave.time.start, wave.time.stop);
      continue;
    }

    ImGui::Text("(active) wave: %0.1f %0.1f", wave.time.start, wave.time.stop);
    ImGui::Text("(active) cooldown: %f", cooldown_c.time);

    const RandomSpawnerWaveKey wave_key{
      .idx_in_random_spawner = wave.on_disk_index,
      .idx_in_random_spawner_waves = 0,
    };
    const auto has_wave_data = live_spawn_data_c.randomspawner_data.contains(wave_key);
    if (!has_wave_data)
      live_spawn_data_c.randomspawner_data[wave_key] = {};

    const auto spawned = live_spawn_data_c.randomspawner_data[wave_key].spawned;
    ImGui::Text("Enemy %s spawned: %i", wave.keys[wave.chosen_key_idx].c_str(), spawned);
  }
}

void
update_ui_debug_spawner_system(entt::registry& r)
{
#if defined(_DEBUG)
  ZoneScoped;
#endif
  auto& menu_c = get_first_component<SINGLE_DebugMenuBar>(r);
  auto ui_state = gesert_menubar_state(menu_c, "Spawners_Data");
  if (!ui_state.enabled)
    return;

  GET_FIRST_OR_RETURN(SurviveTimerComponent, r, timer_e, timer_c);
  GET_FIRST_OR_RETURN(SINGLE_OnDiskSpawners, r, disk_spawn_data_e, disk_spawn_data_c);

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

  update_ui_wave_spawner(r, seconds_from_start);
  update_ui_enemy_spawner(r, seconds_from_start);
  update_ui_random_spawner(r, seconds_from_start);

  ImGui::End();
}

} // namespace game2d