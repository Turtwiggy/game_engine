#include "pch.hpp"

#include "spawner_helpers.hpp"

#include "engine/actors/actor_helpers.hpp"
#include "engine/entt/helpers.hpp"
#include "engine/maths/grid.hpp"
#include "engine/maths/maths.hpp"
#include "modules/actors/actor_boat/boat_components.hpp"
#include "modules/actors/actor_player/components.hpp"
#include "modules/actors/actor_rock/rock_components.hpp"
#include "modules/core/raws/raws_helpers.hpp"
#include "modules/systems/system_cooldown/components.hpp"
#include "resources/data.hpp"
#include "spawner_components.hpp"

namespace game2d {

SINGLE_OnDiskSpawners
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
  return root.get<SINGLE_OnDiskSpawners>();
};

int
min_to_sec(int min)
{
  return min * 60;
};

std::optional<int>
get_wave_index_from_time(const EnemySpawnsData& data, int seconds_from_start)
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

void
init_spawners(entt::registry& r)
{
  const auto& spawn_c = get_first_component<SINGLE_OnDiskSpawners>(r);

  // Give every wave a cooldown component
  for (int i = 0; i < (int)spawn_c.wave_spawner.size(); i++) {
    const auto spawner_e = create_empty<CooldownComponent>(r);
    auto wave = spawn_c.wave_spawner[i];
    wave.on_disk_index = i;
    r.emplace<EnemyWavesData>(spawner_e, wave);
  }

  // Give every spawner a cooldown component
  for (int i = 0; i < (int)spawn_c.enemy_spawner.size(); i++) {
    const auto spawner_e = create_empty<CooldownComponent>(r);
    auto wave = spawn_c.enemy_spawner[i];
    wave.on_disk_index = i;
    r.emplace<EnemySpawnsData>(spawner_e, wave);
  }
};

static engine::RandomState target_rnd(0);

entt::entity
get_random_player_target(entt::registry& r)
{
  const auto players_view = r.view<PlayerBoatComponent>();
  if (players_view.empty())
    return entt::null;
  const int rnd = engine::rand_det_s(target_rnd.rng, 0, (int)players_view.size());

  auto it = players_view.begin();
  std::advance(it, rnd);
  return *it;
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
  const float angle = engine::rand_det_s(rnd.rng, 0.0f, 2.0f * engine::PI);

  // generate a random distance from 0 to radius
  const float distance = engine::rand_det_s(rnd.rng, radius_min, radius_max);

  const auto dir = engine::angle_radians_to_direction(angle);
  const float spawn_x = center.x + dir.x * distance;
  const float spawn_y = center.y + dir.y * distance;
  return { spawn_x, spawn_y };
};

glm::vec2
rnd_position_in_map_but_not_inside_players_or_islands(entt::registry& r)
{
  const int attempts = 5;
  const float map_x = 800;
  const float map_tilesize = default_map_tilesize;

  const auto get_players_gridpos = [&]() -> std::vector<glm::ivec2> {
    std::vector<glm::ivec2> gridpos;
    for (const auto& [e, player_c] : r.view<const PlayerBoatComponent>().each()) {
      const auto gp = engine::grid::worldspace_to_gridspace(get_position(r, e), map_tilesize);
      gridpos.push_back(gp);
    }
    return gridpos;
  };
  const auto get_islands_gridpos = [&]() -> std::vector<glm::ivec2> {
    std::vector<glm::ivec2> gridpos;

    for (const auto& [e, rock_c, rock_bb_c] : r.view<const RockComponent, const BoundingBoxComponent>().each()) {
      const auto tl = rock_bb_c.tl;
      const auto br = rock_bb_c.br;

      for (float y = tl.y; y < br.y; y += map_tilesize) {
        for (float x = tl.x; x < br.x; x += map_tilesize) {
          const auto gp = engine::grid::worldspace_to_gridspace({ x, y }, map_tilesize);
          gridpos.push_back(gp);
        }
      }
    }
    return gridpos;
  };

  const auto offlimit_a = get_players_gridpos();
  const auto offlimit_b = get_islands_gridpos();
  std::vector<glm::ivec2> offlimit_gridpos;
  offlimit_gridpos.insert(offlimit_gridpos.end(), offlimit_a.begin(), offlimit_a.end());
  offlimit_gridpos.insert(offlimit_gridpos.end(), offlimit_b.begin(), offlimit_b.end());

  for (int i = 0; i < attempts; i++) {
    bool valid = true;

    const auto candidate = rnd_position_around_point(r, { 0, 0 }, 0.0f, map_x);
    const auto gp = engine::grid::worldspace_to_gridspace(candidate, map_tilesize);
    const auto n_gp = engine::grid::get_neighbour_gridpos_with_diagonals({ gp.x, gp.y });

    const auto it = std::find(offlimit_gridpos.begin(), offlimit_gridpos.end(), gp);
    if (it == offlimit_gridpos.end())
      return candidate;
  }

  SDL_Log("spawn error; candidate not found for rnd_position_in_map_but_not_inside_players_or_islands()");
  return { 0, 0 };
};

} // namespace game2d