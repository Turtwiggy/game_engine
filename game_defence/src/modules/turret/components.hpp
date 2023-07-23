#pragma once

#include "actors.hpp"

namespace game2d {

struct TurretComponent
{
  EntityType type_to_spawn = EntityType::actor_bullet;
  float time_between_bullets = 0.25f;
  float time_between_bullets_left = 0.25f;

  bool active = false;
};

} // namespace game2d