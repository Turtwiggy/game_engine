#pragma once

#include "modules/physics/helpers.hpp"

// c++ headers
#include <map>
#include <vector>

namespace game2d {

// -- components

struct VelocityComponent
{
  float x = 0.0f;
  float y = 0.0f;
};

struct CollidableComponent
{
  uint32_t layer_id = 0;
};

// -- singleton components

struct SINGLETON_PhysicsComponent
{
  // registered objects to use with physics system
  std::vector<PhysicsObject> collidable;

  // resulting collisions from all registered objects
  std::map<uint64_t, Collision2D> frame_collisions;

  // persistent buffer, meaning the events enter, stay and exit can be tracked.
  std::map<uint64_t, Collision2D> persistent_collisions;

  // collision states
  std::vector<Collision2D> collision_enter;
  std::vector<Collision2D> collision_stay;
  std::vector<Collision2D> collision_exit;
};

} // namespace game2d