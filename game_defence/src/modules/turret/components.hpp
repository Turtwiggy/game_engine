#pragma once

#include "actors.hpp"

namespace game2d {

struct TurretComponent
{
  // after this cutoff, no longer target enemies
  // tilesize * tilesize * tiles
  int distance2_cutoff = 16 * 16 * 100;

  EntityType type_to_spawn = EntityType::actor_bullet;
  float time_between_bullets = 1.0f;
  float time_between_bullets_left = 1.0f;
};

} // namespace game2d