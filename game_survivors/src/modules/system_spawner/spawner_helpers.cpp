#include "spawner_helpers.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/maths/maths.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/core_renderer/components.hpp"
#include "spawner_components.hpp"

namespace game2d {

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
  exploder.enemy_key = "actor_enemy_exploder";

  EnemySpawnWave wave0;
  EnemySpawnWave wave1;
  EnemySpawnWave wave2;
  EnemySpawnWave wave3;
  EnemySpawnWave wave4;

  wave0.wave_start_seconds = min_to_sec(1); // 19:00
  wave0.wave_end_seconds = min_to_sec(2);   // 18:00

  wave1.wave_start_seconds = min_to_sec(2) + 1; // 18:00
  wave1.wave_end_seconds = min_to_sec(6);       // 14:00

  wave2.wave_start_seconds = min_to_sec(7);
  wave2.wave_end_seconds = min_to_sec(9);

  wave3.wave_start_seconds = min_to_sec(11);
  wave3.wave_end_seconds = min_to_sec(13);

  wave4.wave_start_seconds = min_to_sec(15);
  wave4.wave_end_seconds = min_to_sec(16);

  wave0.hp = 30;
  wave1.hp = 30;
  wave2.hp = 60;
  wave3.hp = 240;
  wave4.hp = 400;

  wave0.max_allowed = 2;
  wave1.max_allowed = 10;
  wave2.max_allowed = 1;
  wave3.max_allowed = 4;
  wave4.max_allowed = 20;

  wave0.number_per_spawn = 1;
  wave1.number_per_spawn = 2;
  wave2.number_per_spawn = 1;
  wave3.number_per_spawn = 2;
  wave4.number_per_spawn = 2;

  wave0.spawn_cooldown = 4;
  wave1.spawn_cooldown = 5;
  wave2.spawn_cooldown = 1;
  wave3.spawn_cooldown = 1;
  wave4.spawn_cooldown = 1;

  exploder.waves.push_back(wave0);
  exploder.waves.push_back(wave1);
  exploder.waves.push_back(wave2);
  exploder.waves.push_back(wave3);
  exploder.waves.push_back(wave4);
  return exploder;
};

EnemySpawnData
melee_enemy_1()
{
  EnemySpawnData data;
  data.enemy_key = "actor_enemy_melee";

  EnemySpawnWave wave0;
  EnemySpawnWave wave1;
  EnemySpawnWave wave2;
  EnemySpawnWave wave3;
  EnemySpawnWave wave4;
  EnemySpawnWave wave5;

  wave0.wave_start_seconds = min_to_sec(0);
  wave0.wave_end_seconds = min_to_sec(1);
  wave1.wave_start_seconds = min_to_sec(1) + 1;
  wave1.wave_end_seconds = min_to_sec(2);
  wave2.wave_start_seconds = min_to_sec(2) + 1;
  wave2.wave_end_seconds = min_to_sec(6);
  wave3.wave_start_seconds = min_to_sec(6) + 1;
  wave3.wave_end_seconds = min_to_sec(8);
  wave4.wave_start_seconds = min_to_sec(8);
  wave4.wave_end_seconds = min_to_sec(10);
  wave5.wave_start_seconds = min_to_sec(16);
  wave5.wave_end_seconds = min_to_sec(18);

  wave0.hp = 24;
  wave1.hp = 24;
  wave2.hp = 30;
  wave3.hp = 60;
  wave4.hp = 80;
  wave5.hp = 250;

  wave0.max_allowed = 20;
  wave1.max_allowed = 50;
  wave2.max_allowed = 200;
  wave3.max_allowed = 400;
  wave4.max_allowed = 600;
  wave5.max_allowed = 600;

  wave0.number_per_spawn = 4;
  wave1.number_per_spawn = 10;
  wave2.number_per_spawn = 7;
  wave3.number_per_spawn = 12;
  wave4.number_per_spawn = 16;
  wave5.number_per_spawn = 26;

  wave0.spawn_cooldown = 3;
  wave1.spawn_cooldown = 4;
  wave2.spawn_cooldown = 2;
  wave3.spawn_cooldown = 2;
  wave4.spawn_cooldown = 1;
  wave5.spawn_cooldown = 1;

  data.waves.push_back(wave0);
  data.waves.push_back(wave1);
  data.waves.push_back(wave2);
  data.waves.push_back(wave3);
  data.waves.push_back(wave4);
  data.waves.push_back(wave5);

  return data;
};

EnemySpawnData
melee_enemy_2()
{
  EnemySpawnData data;
  data.enemy_key = "actor_enemy_melee_2";

  EnemySpawnWave wave0;
  EnemySpawnWave wave1;
  EnemySpawnWave wave2;
  EnemySpawnWave wave3;

  wave0.wave_start_seconds = min_to_sec(10) + 5;
  wave0.wave_end_seconds = min_to_sec(11);
  wave1.wave_start_seconds = min_to_sec(11) + 1;
  wave1.wave_end_seconds = min_to_sec(13);
  wave2.wave_start_seconds = min_to_sec(13);
  wave2.wave_end_seconds = min_to_sec(14) + 55;
  wave3.wave_start_seconds = min_to_sec(18);
  wave3.wave_end_seconds = min_to_sec(19) + 59;

  wave0.hp = 200;
  wave1.hp = 250;
  wave2.hp = 400;
  wave3.hp = 500;

  wave0.max_allowed = 30;
  wave1.max_allowed = 100;
  wave2.max_allowed = 300;
  wave3.max_allowed = 300;

  wave0.number_per_spawn = 3;
  wave1.number_per_spawn = 5;
  wave2.number_per_spawn = 14;
  wave3.number_per_spawn = 20;

  wave0.spawn_cooldown = 1;
  wave1.spawn_cooldown = 1;
  wave2.spawn_cooldown = 1;
  wave3.spawn_cooldown = 1;

  data.waves.push_back(wave0);
  data.waves.push_back(wave1);
  data.waves.push_back(wave2);
  data.waves.push_back(wave3);

  return data;
};

EnemySpawnData
projectile_enemy()
{
  EnemySpawnData data;
  data.enemy_key = "actor_enemy_projectile";

  EnemySpawnWave wave0;
  EnemySpawnWave wave1;
  EnemySpawnWave wave2;
  EnemySpawnWave wave3;

  wave0.wave_start_seconds = min_to_sec(6);
  wave0.wave_end_seconds = min_to_sec(8);
  wave1.wave_start_seconds = min_to_sec(11);
  wave1.wave_end_seconds = min_to_sec(13);
  wave2.wave_start_seconds = min_to_sec(15);
  wave2.wave_end_seconds = min_to_sec(16);
  wave3.wave_start_seconds = min_to_sec(18);
  wave3.wave_end_seconds = min_to_sec(19) + 59;

  wave0.hp = 400;
  wave1.hp = 200;
  wave2.hp = 300;
  wave3.hp = 500;

  wave0.max_allowed = 2;
  wave1.max_allowed = 12;
  wave2.max_allowed = 16;
  wave3.max_allowed = 10;

  wave0.number_per_spawn = 2;
  wave1.number_per_spawn = 2;
  wave2.number_per_spawn = 2;
  wave3.number_per_spawn = 1;

  wave0.spawn_cooldown = 10;
  wave1.spawn_cooldown = 1;
  wave2.spawn_cooldown = 1;
  wave3.spawn_cooldown = 1;

  return data;
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

static engine::RandomState target_rnd(0);

entt::entity
get_random_player_target(entt::registry& r)
{
  const auto& players_view = r.view<PlayerComponent>();
  if (players_view.size() == 0)
    return entt::null;
  const int rnd = engine::rand_det_s(target_rnd.rng, 0, int(players_view.size()));
  return players_view[rnd];
};

glm::ivec2
rnd_position_around_point(entt::registry& r, const glm::ivec2 center, float radius)
{
  const auto& ri = get_first_component<SINGLE_RendererInfo>(r);
  static engine::RandomState rnd(0);
  const float rnd_val_0 = engine::rand_01(rnd.rng);

  // generate a random angle 0 to 2PI
  float angle = engine::rand_det_s(rnd.rng, 0.0f, 2.0f * engine::PI);

  // generate a random distance outside the radius
  // float radius = std::max(ri.viewport_size_render_at.x, ri.viewport_size_render_at.y);
  float distance = radius;

  const auto dir = engine::angle_radians_to_direction(angle);
  float spawn_x = center.x + dir.x * distance;
  float spawn_y = center.y + dir.y * distance;
  return { spawn_x, spawn_y };
};

} // namespace game2d