#pragma once

#include "spawner_components.hpp"

#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include <optional>

namespace game2d {

int
min_to_sec(int min);

// todo: replace below with configs
EnemySpawnData
exploder_data();
EnemySpawnData
melee_enemy_1();
EnemySpawnData
melee_enemy_2();
EnemySpawnData
projectile_enemy();

std::optional<EnemySpawnWave>
get_wave_from_time(const EnemySpawnData& data, int seconds_from_start);

entt::entity
get_random_player_target(entt::registry& r);

glm::ivec2
rnd_position_around_point(entt::registry& r, const glm::ivec2 center, float radius = 500);

} // namespace game2d