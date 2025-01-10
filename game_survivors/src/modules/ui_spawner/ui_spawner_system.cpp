#include "ui_spawner_system.hpp"
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

  EnemySpawnWave wave0;
  wave0.wave_start_seconds = min_to_sec(1); // 19:00
  wave0.wave_end_seconds = min_to_sec(2);   // 18:00
  wave0.hp = 30;
  wave0.max_allowed = 2;
  wave0.number_per_spawn = 1;
  wave0.spawn_cooldown = 4;

  EnemySpawnWave wave1;
  wave1.wave_start_seconds = min_to_sec(2); // 18:00
  wave1.wave_end_seconds = min_to_sec(6);   // 14:00
  wave0.hp = 30;
  wave0.max_allowed = 10;
  wave0.number_per_spawn = 1;
  wave0.spawn_cooldown = 1;

  EnemySpawnWave wave2;
  wave1.wave_start_seconds = min_to_sec(2); // 18:00
  wave1.wave_end_seconds = min_to_sec(6);   // 14:00
  wave0.hp = 30;
  wave0.max_allowed = 10;
  wave0.number_per_spawn = 1;
  wave0.spawn_cooldown = 1;

  EnemySpawnData exploder;
  exploder.waves.push_back(wave0);
  exploder.waves.push_back(wave1);
  exploder.waves.push_back(wave2);

  return exploder;
};

std::optional<EnemySpawnWave>
get_wave_from_time(const EnemySpawnData& data, int minutes, int seconds)
{
  int total_seconds = minutes * 60 + seconds;

  for (const auto& wave : data.waves) {
    if (total_seconds < wave.wave_end_seconds)
      return wave;
  }

  return std::nullopt;
};

void
update_ui_spawner_system(entt::registry& r)
{
  GET_FIRST_OR_RETURN(SurviveTimerComponent, r, timer_e, timer_c);
  const auto& cooldown_c = r.get<CooldownComponent>(timer_e);
  const int minutes = static_cast<int>(cooldown_c.time) / 60;
  const int seconds = static_cast<int>(cooldown_c.time) % 60;

  ImGui::Begin("DebugSpawner");

  static EnemySpawnData sploder = exploder_data();

  // Debug wave
  const auto wave_opt = get_wave_from_time(sploder, minutes, seconds);
  if (wave_opt.has_value()) {
    const auto& w = wave_opt.value();
    ImGui::SeparatorText("Wave");
    ImGui::Text("wave_start_seconds %i", w.wave_start_seconds);
    ImGui::Text("wave_end_seconds %i", w.wave_end_seconds);
    ImGui::Text("hp %i", w.hp);
    ImGui::Text("max_allowed %i", w.max_allowed);
    ImGui::Text("number_per_spawn %i", w.number_per_spawn);
    ImGui::Text("spawn_cooldown %i", w.spawn_cooldown);
  }

  ImGui::End();
}

} // namespace game2d