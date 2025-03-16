#pragma once

#include <entt/fwd.hpp>
#include <nlohmann/detail/macro_scope.hpp>
#include <nlohmann/json.hpp>

namespace game2d {

struct TimeSpan
{
  int start = 0;
  int stop = 0;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(TimeSpan, start, stop);
};

struct WaveData
{
  float hp = 30;
  int max = 2; // at any one time
  int num_per_spawn = 1;
  std::optional<int> num_per_wave = std::nullopt; // max to spawn for that wave
  // std::optional<float> spawn_cooldown = 2.5f;

  friend void to_json(nlohmann ::json& j, const WaveData& val)
  {
    j["hp"] = val.hp;
    j["max"] = val.max;
    j["num_per_spawn"] = val.num_per_spawn;
    if (val.num_per_wave.has_value())
      j["num_per_wave"] = val.num_per_wave.value();
    // if (val.spawn_cooldown.has_value())
    //   j["spawn_cooldown"] = val.spawn_cooldown.value();
  }

  friend void from_json(const nlohmann ::json& j, WaveData& val)
  {
    j.at("hp").get_to(val.hp);
    j.at("max").get_to(val.max);
    j.at("num_per_spawn").get_to(val.num_per_spawn);
    if (j.contains("num_per_wave"))
      j.at("num_per_wave").get_to(val.num_per_wave.emplace());
    // if (j.contains("spawn_cooldown"))
    //   j.at("spawn_cooldown").get_to(val.spawn_cooldown.emplace());
  };
};

//
// Used by "waves",
// when we want to specify what enemies a wave will contain
//
struct WaveEnemyType
{
  std::string key;
  WaveData data;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(WaveEnemyType, key, data);
};

//
// Used by "spawns",
// when we want to specify an enemy type to spawn for a period of time
//
struct WaveEnemyTime
{
  TimeSpan span;
  WaveData data;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(WaveEnemyTime, span, data);
};

//
// Waves: A wave consists of multiple enemy types
//
struct EnemyWavesData
{
  std::string name;
  TimeSpan time;
  std::vector<WaveEnemyType> enemies;

  int on_disk_index = 0; // position in SINGLE_Spawners

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(EnemyWavesData, name, time, enemies)
};

//
// Spawner: A spawner spawns one enemy over for multiple waves
//
struct EnemySpawnsData
{
  std::string key = "actor_enemy_melee_1";
  std::vector<WaveEnemyTime> waves;

  int on_disk_index = 0; // position in SINGLE_Spawners

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(EnemySpawnsData, key, waves)
};

struct SINGLE_OnDiskSpawners
{
  std::vector<EnemyWavesData> wave_spawner;
  std::vector<EnemySpawnsData> enemy_spawner;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(SINGLE_OnDiskSpawners, wave_spawner, enemy_spawner);
};

//
//
//

struct WaveSpawnerWaveKey
{
  int idx_in_wave_spawner = 0;
  int idx_in_wave_spawner_enemy = 0;

  auto operator<=>(const WaveSpawnerWaveKey&) const = default;
};

struct EnemySpawnerWaveKey
{
  int idx_in_enemy_spawner = 0;       // idx in enemy_spawner
  int idx_in_enemy_spawner_waves = 0; // idx in enemy_spawner[0].waves

  auto operator<=>(const EnemySpawnerWaveKey&) const = default;
};

struct WaveLiveData
{
  int spawned = 0;
};

struct enemyspawnerwavekey_hash
{
  std::size_t operator()(const EnemySpawnerWaveKey& key) const
  {
    const auto hash1 = std::hash<int>{}(key.idx_in_enemy_spawner);
    const auto hash2 = std::hash<int>{}(key.idx_in_enemy_spawner_waves);
    return hash1 ^ (hash2 << 1);
  }
};

struct wavespawnerwavekey_hash
{
  std::size_t operator()(const WaveSpawnerWaveKey& key) const
  {
    const auto hash1 = std::hash<int>{}(key.idx_in_wave_spawner);
    const auto hash2 = std::hash<int>{}(key.idx_in_wave_spawner_enemy);
    return hash1 ^ (hash2 << 1);
  }
};

struct SpawnerLiveData
{
  std::unordered_map<EnemySpawnerWaveKey, WaveLiveData, enemyspawnerwavekey_hash> enemyspawner_data;
  std::unordered_map<WaveSpawnerWaveKey, WaveLiveData, wavespawnerwavekey_hash> wavespawner_data;
};

} // namespace game2d