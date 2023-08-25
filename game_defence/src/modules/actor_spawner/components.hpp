#pragma once

#include "actors.hpp"

namespace game2d {

struct SpawnerComponent
{
  EntityType type_to_spawn = EntityType::actor_enemy;

  bool continuous_spawn = true;
  float time_between_spawns = 1.0f;
  float time_between_spawns_left = 1.0f;
};

} // namespace game2d