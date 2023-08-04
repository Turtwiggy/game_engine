#pragma once

#include "actors.hpp"

#include <entt/entt.hpp>

namespace game2d {

struct TurretComponent
{
  EntityType type_to_spawn = EntityType::actor_bullet;
  float time_between_bullets = 0.25f;
  float time_between_bullets_left = 0.25f;

  bool active = true;
};

struct ClosestInfo
{
  entt::entity e = entt::null;
  int distance2 = std::numeric_limits<int>::max();
};

} // namespace game2d