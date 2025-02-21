#include "spawner_helpers.hpp"

#include "engine/entt/helpers.hpp"
#include "engine/maths/maths.hpp"
#include "modules/actor_enemy/components.hpp"
#include "modules/actor_player/components.hpp"
#include "modules/core_raws/raws_components.hpp"
#include "modules/core_raws/raws_helpers.hpp"
#include "modules/core_renderer/components.hpp"
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

std::optional<EnemySpawnWave>
get_wave_from_time(const EnemySpawnData& data, int seconds_from_start)
{
  for (const auto& wave : data.waves) {
    const bool in_lower_bound = seconds_from_start >= min_to_sec(wave.span.start);
    const bool in_upper_bound = seconds_from_start < min_to_sec(wave.span.stop);
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
rnd_position_around_point(entt::registry& r, const glm::ivec2 center)
{
  const auto& ri = get_first_component<SINGLE_RendererInfo>(r);

#if defined(_DEBUG)
  // static engine::RandomState rnd(0);
  static engine::RandomState rnd(engine::get_system_time_for_seed());
#else
  static engine::RandomState rnd(engine::get_system_time_for_seed());
#endif

  const float rnd_val_0 = engine::rand_01(rnd.rng);

  // generate a random angle 0 to 2PI
  float angle = engine::rand_det_s(rnd.rng, 0.0f, 2.0f * engine::PI);

  // generate a random distance outside the radius
  float radius = std::max(ri.viewport_size_render_at.x, ri.viewport_size_render_at.y);
  float distance = radius;

  const auto dir = engine::angle_radians_to_direction(angle);
  float spawn_x = center.x + dir.x * distance;
  float spawn_y = center.y + dir.y * distance;
  return { spawn_x, spawn_y };
};

std::unordered_map<std::string, int>
get_live_enemies_map(entt::registry& r)
{
  // How many of each enemies do we currently have?
  const auto& enemies_view = r.view<EnemyComponent, ItemKey>();

  std::unordered_map<std::string, int> enemy_to_amount;
  for (const auto& [e, enemy_c, item_c] : enemies_view.each())
    enemy_to_amount[item_c.key] += 1;
  return enemy_to_amount;
};

} // namespace game2d