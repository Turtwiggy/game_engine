#pragma once

#include "spawner_components.hpp"

#include <entt/fwd.hpp>
#include <glm/fwd.hpp>

#include <optional>

namespace game2d {

int
min_to_sec(int min);

SINGLE_Spawners
load_spawns(std::string directory);

void
init_spawners(entt::registry& r);

std::optional<int>
get_wave_index_from_time(const EnemySpawnData& data, int seconds_from_start);

entt::entity
get_random_player_target(entt::registry& r);

glm::vec2
rnd_position_around_point(entt::registry& r, const glm::ivec2 center, float radius_min, float radius_max);

glm::vec2
rnd_position_in_map_but_not_inside_players(entt::registry& r);

} // namespace game2d