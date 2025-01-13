#include "ui_debug_spawner_system.hpp"
#include "engine/entt/helpers.hpp"
#include "modules/system_cooldown/components.hpp"
#include "modules/ui_survive_timer/ui_survive_timer_components.hpp"

#include <imgui.h>

namespace game2d {

struct EnemySpawnWave
{
  int wave_start_seconds = 60;
  int wave_end_seconds = 120;
  int hp = 30;
  int max_allowed = 2;
  int number_per_spawn = 1;
  int spawn_cooldown = 4;
};

struct EnemySpawnData
{
  std::vector<EnemySpawnWave> waves;
};

int
min_to_sec(int min)
{
  return min * 60;
};

EnemySpawnData
exploder_data()
{
  // actor_enemy_exploder
  EnemySpawnData exploder;

  {
    EnemySpawnWave wave;
    wave.wave_start_seconds = min_to_sec(1); // 19:00
    wave.wave_end_seconds = min_to_sec(2);   // 18:00
    wave.hp = 30;
    wave.max_allowed = 2;
    wave.number_per_spawn = 1;
    wave.spawn_cooldown = 4;

    exploder.waves.push_back(wave);
  }

  {
    EnemySpawnWave wave;
    wave.wave_start_seconds = min_to_sec(2); // 18:00
    wave.wave_end_seconds = min_to_sec(6);   // 14:00
    wave.hp = 30;
    wave.max_allowed = 10;
    wave.number_per_spawn = 1;
    wave.spawn_cooldown = 1;

    exploder.waves.push_back(wave);
  }

  {
    EnemySpawnWave wave;
    wave.wave_start_seconds = min_to_sec(2); // 18:00
    wave.wave_end_seconds = min_to_sec(6);   // 14:00
    wave.hp = 30;
    wave.max_allowed = 10;
    wave.number_per_spawn = 1;
    wave.spawn_cooldown = 1;

    exploder.waves.push_back(wave);
  }

  return exploder;
};

std::optional<EnemySpawnWave>
get_wave_from_time(const EnemySpawnData& data, int seconds_from_start)
{
  for (const auto& wave : data.waves) {
    const bool in_lower_bound = seconds_from_start >= wave.wave_start_seconds;
    const bool in_upper_bound = seconds_from_start < wave.wave_end_seconds;
    if (in_lower_bound && in_upper_bound)
      return wave;
  }

  return std::nullopt;
};

void
update_ui_debug_spawner_system(entt::registry& r)
{
  GET_FIRST_OR_RETURN(SurviveTimerComponent, r, timer_e, timer_c);
  const auto& cooldown_c = r.get<CooldownComponent>(timer_e);

  ImGui::Begin("DebugSpawner");

  const int minutes = static_cast<int>(cooldown_c.time) / 60;
  const int seconds = static_cast<int>(cooldown_c.time) % 60;
  const int seconds_from_start = cooldown_c.time_max - minutes * 60 + seconds;
  ImGui::Text("Seconds from start: %i", seconds_from_start);

  static EnemySpawnData sploder = exploder_data();

  // Debug wave
  const auto wave_opt = get_wave_from_time(sploder, seconds_from_start);
  if (wave_opt.has_value()) {
    const auto& w = wave_opt.value();
    ImGui::SeparatorText("Wave");
    ImGui::Text("wave_start_seconds %i", w.wave_start_seconds);
    ImGui::Text("wave_end_seconds %i", w.wave_end_seconds);
    ImGui::Text("hp %i", w.hp);
    ImGui::Text("max_allowed %i", w.max_allowed);
    ImGui::Text("number_per_spawn %i", w.number_per_spawn);
    ImGui::Text("spawn_cooldown %i", w.spawn_cooldown);
  } else
    ImGui::Text("Sploder; no wave data");

  ImGui::End();
}

} // namespace game2d