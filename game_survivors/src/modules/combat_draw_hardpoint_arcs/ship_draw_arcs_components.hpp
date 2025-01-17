#pragma once

#include <entt/entt.hpp>

namespace game2d {

struct ShipArcComponent
{
  // 0 = along x axis. +90 = 'up', +270 = 'down'
  float forward = 0;
  float arc_mid = 55;
  float arc = 224;
  float x_rel_tl = 22.5;
  float y_rel_tl = 4.5;
};

} // namespace game2d