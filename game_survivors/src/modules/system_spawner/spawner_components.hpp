#pragma once

#include <entt/fwd.hpp>
#include <nlohmann/detail/macro_scope.hpp>
#include <nlohmann/json.hpp>

#include <unordered_map>

namespace game2d {

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
  int max = 2; // at any one time
  int num_per_spawn = 1;
  std::optional<int> num_per_wave = std::nullopt; // max to spawn for that wave
  std::optional<float> spawn_cooldown = 2.5f;

  friend void to_json(nlohmann ::json& j, const EnemySpawnWave& val)
  {
    j["span"] = val.span;
    j["hp"] = val.hp;
    j["max"] = val.max;
    j["num_per_spawn"] = val.num_per_spawn;
    if (val.num_per_wave.has_value())
      j["num_per_wave"] = val.num_per_wave.value();
    if (val.spawn_cooldown.has_value())
      j["spawn_cooldown"] = val.spawn_cooldown.value();
  }
  friend void from_json(const nlohmann ::json& j, EnemySpawnWave& val)
  {
    j.at("span").get_to(val.span);
    j.at("hp").get_to(val.hp);
    j.at("max").get_to(val.max);
    j.at("num_per_spawn").get_to(val.num_per_spawn);
    if (j.contains("num_per_wave"))
      j.at("num_per_wave").get_to(val.num_per_wave.emplace());
    if (j.contains("spawn_cooldown"))
      j.at("spawn_cooldown").get_to(val.spawn_cooldown.emplace());
  };
};

struct EnemySpawnData
{
  std::string key = "actor_enemy_exploder";
  std::vector<EnemySpawnWave> waves;

  // index to be set when instantiated in entt,
  // but it's the position that this data is
  // in the SINGLE_Spawners spawns data
  int on_disk_index = 0;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(EnemySpawnData, key, waves)
};

struct SINGLE_Spawners
{
  std::vector<EnemySpawnData> spawns;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(SINGLE_Spawners, spawns);
};

struct WaveKey
{
  int on_disk_spawns_index = 0; // spawns are per-enemy
  int on_disk_waves_index = 0;  // how many waves does the enemy have
};

bool
operator==(const WaveKey& a, const WaveKey& b);

struct WaveLiveData
{
  int spawned = 0;
};

struct wavekey_hash
{
  std::size_t operator()(const WaveKey& key) const
  {
    const auto hash1 = std::hash<int>{}(key.on_disk_spawns_index);
    const auto hash2 = std::hash<int>{}(key.on_disk_waves_index);
    return hash1 ^ (hash2 << 1);
  }
};

struct SINGLE_SpawnerLiveData
{
  std::unordered_map<WaveKey, WaveLiveData, wavekey_hash> data;
};

} // namespace game2d