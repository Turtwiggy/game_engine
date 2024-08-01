#pragma once

#include "actors.hpp"
#include "physics/components.hpp"

#include <vector>

namespace game2d {

struct SpawnerComponent
{
  std::vector<EntityType> types_to_spawn{};

  bool continuous_spawn = true;

  bool spawn_in_boundingbox = false;
  AABB spawn_area;
};

struct OnlySpawnInRangeOfAnyPlayerComponent
{
  float distance2 = 50000; // sqrt(distance2) is the x or y distance
};

} // namespace game2d