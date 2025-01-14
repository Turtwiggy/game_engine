#pragma once

#include <entt/entt.hpp>

namespace game2d {

// struct SpawnerComponent
// {
//   bool placeholder = true;
// };

struct EnemySpawnWave
{
  int wave_start_seconds = 0;
  int wave_end_seconds = 20 * 60;
  int hp = 30;
  int max_allowed = 2;
  int number_per_spawn = 1;
  int spawn_cooldown = 4;
};

struct EnemySpawnData
{
  std::string enemy_key = "actor_enemy_exploder";
  std::vector<EnemySpawnWave> waves;
};

// // walk up and hit
// struct AiMelee
// {
//   bool placeholder = true;
// };

// // walk up and explode
// struct AiExploder
// {
//   bool placeholder = true;
// };

// // get to roughly X distance,
// // then shoot a projectile
// struct AiProjectile
// {
//   bool placeholder = true;
// };

} // namespace game2d