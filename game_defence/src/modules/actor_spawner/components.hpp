#pragma once

#include "actors.hpp"

namespace game2d {

struct SpawnerComponent
{
  EntityType type_to_spawn = EntityType::enemy_grunt;

  bool continuous_spawn = true;
};

} // namespace game2d