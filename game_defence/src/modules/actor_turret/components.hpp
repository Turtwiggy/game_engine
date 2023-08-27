#pragma once

#include "actors.hpp"

#include <entt/entt.hpp>

namespace game2d {

// turret currently basically means "shoot at nearest"
struct TurretComponent
{
  bool active = true;
};

struct ClosestInfo
{
  entt::entity e = entt::null;
  int distance2 = std::numeric_limits<int>::max();
};

} // namespace game2d