#pragma once

#include "spawner_components.hpp"

#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include <optional>

namespace game2d {

SINGLE_Spawners
load_spawns(std::string directory);

int
min_to_sec(int min);

std::optional<EnemySpawnWave>
get_wave_from_time(const EnemySpawnData& data, int seconds_from_start);

entt::entity
get_random_player_target(entt::registry& r);

glm::ivec2
rnd_position_around_point(entt::registry& r, const glm::ivec2 center);

std::unordered_map<std::string, int>
get_live_enemies_map(entt::registry& r);

} // namespace game2d