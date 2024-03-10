#pragma once

#include "actors.hpp"

#include <vector>

namespace game2d {

struct SpawnerComponent
{
  std::vector<EntityType> types_to_spawn{ EntityType::enemy_grunt };

  bool continuous_spawn = true;
};

struct OnlySpawnInRangeOfAnyPlayerComponent
{
  float distance2 = 50000; // sqrt(distance2) is the x or y distance
};

} // namespace game2d