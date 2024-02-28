#pragma once

#include "actors.hpp"

namespace game2d {

struct SpawnerComponent
{
  EntityType type_to_spawn = EntityType::enemy_grunt;

  bool continuous_spawn = true;
};

struct OnlySpawnInRangeOfAnyPlayerComponent
{
  float distance2 = 500000; // sqrt(distance2) is the x or y distance
};

} // namespace game2d