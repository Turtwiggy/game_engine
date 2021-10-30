#pragma once

#include "helpers/physics_layers.hpp"

namespace game2d {

struct VelocityComponent
{
  float x = 0.0f;
  float y = 0.0f;
};

struct CollidableComponent
{
  CollisionLayer layer;
};

} // namespace game2d