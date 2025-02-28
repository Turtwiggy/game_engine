#include "spawner_helpers.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/maths/maths.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/core_raws/raws_helpers.hpp"
#include "modules/system_cooldown/components.hpp"
#include "spawner_components.hpp"

#include <fstream>
#include <sstream>

namespace game2d {

SINGLE_Spawners
load_spawns(std::string filepath)
{
  SDL_Log("loading spawns... %s", filepath.c_str());

  // load from disk
  std::ifstream t(filepath);
  std::stringstream buffer;
  buffer << t.rdbuf();
  const std::string data_with_comments = buffer.str();

  // remove comments from .jsonc file
  std::istringstream stream(data_with_comments);
  std::ostringstream output;
  std::string line;
  while (std::getline(stream, line)) {
    std::string cleaned_line = remove_comment(line);
    output << cleaned_line << "\n";
  }

  const std::string string_without_comments = output.str();
  nlohmann::json root = nlohmann::json::parse(string_without_comments);
  return root.get<SINGLE_Spawners>();
};

int
min_to_sec(int min)
{
  return min * 60;
};

void
init_spawners(entt::registry& r)
{
  create_empty<SINGLE_SpawnerLiveData>(r);

  const auto& spawn_c = get_first_component<SINGLE_Spawners>(r);
  for (auto i = 0; i < (int)spawn_c.spawns.size(); i++) {
    const auto spawner_e = create_empty<CooldownComponent>(r);

    EnemySpawnData copy_data = spawn_c.spawns[i];
    copy_data.on_disk_index = i;
    r.emplace<EnemySpawnData>(spawner_e, copy_data);
  }
}

std::optional<int>
get_wave_index_from_time(const EnemySpawnData& data, int seconds_from_start)
{
  for (size_t i = 0; i < data.waves.size(); i++) {
    const auto& wave = data.waves[i];
    const bool in_lower_bound = seconds_from_start >= min_to_sec(wave.span.start);
    const bool in_upper_bound = seconds_from_start < min_to_sec(wave.span.stop);
    if (in_lower_bound && in_upper_bound)
      return (int)i;
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

glm::vec2
rnd_position_around_point(entt::registry& r, const glm::ivec2 center, float radius_min, float radius_max)
{
#if defined(_DEBUG)
  static engine::RandomState rnd(0);
  // static engine::RandomState rnd(engine::get_system_time_for_seed());
#else
  static engine::RandomState rnd(engine::get_system_time_for_seed());
#endif

  const float rnd_val_0 = engine::rand_01(rnd.rng);

  // generate a random angle 0 to 2PI
  float angle = engine::rand_det_s(rnd.rng, 0.0f, 2.0f * engine::PI);

  // generate a random distance from 0 to radius
  float distance = engine::rand_det_s(rnd.rng, radius_min, radius_max);

  const auto dir = engine::angle_radians_to_direction(angle);
  float spawn_x = center.x + dir.x * distance;
  float spawn_y = center.y + dir.y * distance;
  return { spawn_x, spawn_y };
};

glm::vec2
rnd_position_in_map_but_not_inside_players(entt::registry& r)
{
  int attempts = 3;
  auto candidate = rnd_position_around_point(r, { 0, 0 }, 0.0f, 700.0f);

  for (int i = 0; i < attempts; i++) {
    bool valid = true;

    auto view = r.view<const PlayerComponent>();
    for (const auto& [e, player_c] : view.each()) {
      auto d = get_position(r, e) - candidate;
      const float d2 = d.x * d.x + d.y * d.y;
      // SDL_Log("d2: %f", d2);

      constexpr int buffer_size_sqr = 32 * 32;
      constexpr int player_size_sqr = 32 * 32;
      if (d2 < (player_size_sqr + buffer_size_sqr))
        valid = false; // candidate invalid. try again.
    }

    if (valid)
      break;
    candidate = rnd_position_around_point(r, { 0, 0 }, 0.0f, 700.0f);
  }

  return candidate;
};

} // namespace game2d