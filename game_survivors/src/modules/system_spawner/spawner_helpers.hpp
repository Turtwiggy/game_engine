#pragma once

#include "spawner_components.hpp"

#include <entt/entt.hpp>
#include <glm/glm.hpp>

#include <optional>

namespace game2d {

int
min_to_sec(int min);

//
// todo: replace below with configs
//

EnemySpawnData
melee_enemy_1();

EnemySpawnData
melee_enemy_2();

EnemySpawnData
melee_enemy_3();

EnemySpawnData
exploder_data();

EnemySpawnData
projectile_enemy();

EnemySpawnData
swarmlord_enemy();

//
//
//

std::optional<EnemySpawnWave>
get_wave_from_time(const EnemySpawnData& data, int seconds_from_start);

entt::entity
get_random_player_target(entt::registry& r);

glm::ivec2
rnd_position_around_point(entt::registry& r, const glm::ivec2 center);

std::unordered_map<std::string, int>
get_live_enemies_map(entt::registry& r);

} // namespace game2d