#include "spawner_helpers.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/maths/maths.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/renderer/components.hpp"
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
    wave.wave_start_seconds = min_to_sec(7);
    wave.wave_end_seconds = min_to_sec(9);
    wave.hp = 60;
    wave.max_allowed = 1;
    wave.number_per_spawn = 1;
    wave.spawn_cooldown = 1;

    exploder.waves.push_back(wave);
  }

  {
    EnemySpawnWave wave;
    wave.wave_start_seconds = min_to_sec(11);
    wave.wave_end_seconds = min_to_sec(13);
    wave.hp = 240;
    wave.max_allowed = 4;
    wave.number_per_spawn = 2;
    wave.spawn_cooldown = 1;

    exploder.waves.push_back(wave);
  }

  {
    EnemySpawnWave wave;
    wave.wave_start_seconds = min_to_sec(15);
    wave.wave_end_seconds = min_to_sec(16);
    wave.hp = 400;
    wave.max_allowed = 20;
    wave.number_per_spawn = 2;
    wave.spawn_cooldown = 1;

    exploder.waves.push_back(wave);
  }

  return exploder;
};

EnemySpawnData
melee_enemy_1()
{
  EnemySpawnData data;
  data.enemy_key = "actor_enemy_melee";

  {
    EnemySpawnWave wave;
    wave.wave_start_seconds = min_to_sec(0);
    wave.wave_end_seconds = min_to_sec(1);
    wave.hp = 24;
    wave.max_allowed = 20;
    wave.number_per_spawn = 4;
    wave.spawn_cooldown = 3;

    data.waves.push_back(wave);
  }

  {
    EnemySpawnWave wave;
    wave.wave_start_seconds = min_to_sec(1);
    wave.wave_end_seconds = min_to_sec(2);
    wave.hp = 24;
    wave.max_allowed = 50;
    wave.number_per_spawn = 10;
    wave.spawn_cooldown = 4;

    data.waves.push_back(wave);
  }

  {
    EnemySpawnWave wave;
    wave.wave_start_seconds = min_to_sec(2);
    wave.wave_end_seconds = min_to_sec(6);
    wave.hp = 30;
    wave.max_allowed = 200;
    wave.number_per_spawn = 7;
    wave.spawn_cooldown = 2;

    data.waves.push_back(wave);
  }

  {
    EnemySpawnWave wave;
    wave.wave_start_seconds = min_to_sec(6);
    wave.wave_end_seconds = min_to_sec(8);
    wave.hp = 60;
    wave.max_allowed = 400;
    wave.number_per_spawn = 12;
    wave.spawn_cooldown = 2;

    data.waves.push_back(wave);
  }

  {
    EnemySpawnWave wave;
    wave.wave_start_seconds = min_to_sec(8);
    wave.wave_end_seconds = min_to_sec(10);
    wave.hp = 80;
    wave.max_allowed = 600;
    wave.number_per_spawn = 16;
    wave.spawn_cooldown = 1;

    data.waves.push_back(wave);
  }

  {
    EnemySpawnWave wave;
    wave.wave_start_seconds = min_to_sec(16);
    wave.wave_end_seconds = min_to_sec(18);
    wave.hp = 250;
    wave.max_allowed = 600;
    wave.number_per_spawn = 26;
    wave.spawn_cooldown = 1;

    data.waves.push_back(wave);
  }

  return data;
};

EnemySpawnData
melee_enemy_2()
{
  EnemySpawnData data;
  data.enemy_key = "actor_enemy_melee_2";

  {
    EnemySpawnWave wave;
    wave.wave_start_seconds = min_to_sec(10) + 5;
    wave.wave_end_seconds = min_to_sec(11);
    wave.hp = 200;
    wave.max_allowed = 30;
    wave.number_per_spawn = 3;
    wave.spawn_cooldown = 1;

    data.waves.push_back(wave);
  }

  {
    EnemySpawnWave wave;
    wave.wave_start_seconds = min_to_sec(11);
    wave.wave_end_seconds = min_to_sec(13);
    wave.hp = 250;
    wave.max_allowed = 100;
    wave.number_per_spawn = 5;
    wave.spawn_cooldown = 1;

    data.waves.push_back(wave);
  }

  {
    EnemySpawnWave wave;
    wave.wave_start_seconds = min_to_sec(13);
    wave.wave_end_seconds = min_to_sec(14) + 55;
    wave.hp = 400;
    wave.max_allowed = 300;
    wave.number_per_spawn = 14;
    wave.spawn_cooldown = 1;

    data.waves.push_back(wave);
  }

  {
    EnemySpawnWave wave;
    wave.wave_start_seconds = min_to_sec(18);
    wave.wave_end_seconds = min_to_sec(19) + 59;
    wave.hp = 500;
    wave.max_allowed = 300;
    wave.number_per_spawn = 20;
    wave.spawn_cooldown = 1;

    data.waves.push_back(wave);
  }

  return data;
};

EnemySpawnData
projectile_enemy()
{
  EnemySpawnData data;
  data.enemy_key = "actor_enemy_projectile";

  {
    EnemySpawnWave wave;
    wave.wave_start_seconds = min_to_sec(6);
    wave.wave_end_seconds = min_to_sec(8);
    wave.hp = 400;
    wave.max_allowed = 2;
    wave.number_per_spawn = 2;
    wave.spawn_cooldown = 10;

    data.waves.push_back(wave);
  }

  {
    EnemySpawnWave wave;
    wave.wave_start_seconds = min_to_sec(11);
    wave.wave_end_seconds = min_to_sec(13);
    wave.hp = 200;
    wave.max_allowed = 12;
    wave.number_per_spawn = 2;
    wave.spawn_cooldown = 1;

    data.waves.push_back(wave);
  }

  {
    EnemySpawnWave wave;
    wave.wave_start_seconds = min_to_sec(15);
    wave.wave_end_seconds = min_to_sec(16);
    wave.hp = 300;
    wave.max_allowed = 16;
    wave.number_per_spawn = 2;
    wave.spawn_cooldown = 1;

    data.waves.push_back(wave);
  }

  {
    EnemySpawnWave wave;
    wave.wave_start_seconds = min_to_sec(18);
    wave.wave_end_seconds = min_to_sec(19) + 59;
    wave.hp = 500;
    wave.max_allowed = 10;
    wave.number_per_spawn = 1;
    wave.spawn_cooldown = 1;

    data.waves.push_back(wave);
  }

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