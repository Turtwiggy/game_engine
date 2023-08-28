#pragma once

#include "actors.hpp"

#include <entt/entt.hpp>

namespace game2d {

// turret currently basically means "shoot at nearest"
struct TurretComponent
{
  bool active = true;
};

} // namespace game2d