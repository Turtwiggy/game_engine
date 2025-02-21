#pragma once

#include <entt/entt.hpp>
#include <nlohmann/detail/macro_scope.hpp>
#include <nlohmann/json.hpp>

namespace game2d {

// struct SpawnerComponent
// {
//   bool placeholder = true;
// };

struct TimeSpan
{
  int start = 0;
  int stop = 0;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(TimeSpan, start, stop);
};

struct EnemySpawnWave
{
  TimeSpan span;
  float hp = 30;
  int max = 2;
  int num_per_spawn = 1;
  std::optional<float> spawn_cooldown = 2.5f;

  friend void to_json(nlohmann ::json& j, const EnemySpawnWave& val)
  {
    j["span"] = val.span;
    j["hp"] = val.hp;
    j["max"] = val.max;
    j["num_per_spawn"] = val.num_per_spawn;
    if (val.spawn_cooldown.has_value())
      j["spawn_cooldown"] = val.spawn_cooldown.value();
  }
  friend void from_json(const nlohmann ::json& j, EnemySpawnWave& val)
  {
    j.at("span").get_to(val.span);
    j.at("hp").get_to(val.hp);
    j.at("max").get_to(val.max);
    j.at("num_per_spawn").get_to(val.num_per_spawn);
    if (j.contains("spawn_cooldown"))
      j.at("spawn_cooldown").get_to(val.spawn_cooldown.emplace());
  };
};

struct EnemySpawnData
{
  std::string key = "actor_enemy_exploder";
  std::vector<EnemySpawnWave> waves;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(EnemySpawnData, key, waves)
};

struct SINGLE_Spawners
{
  std::vector<EnemySpawnData> spawns;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(SINGLE_Spawners, spawns);
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