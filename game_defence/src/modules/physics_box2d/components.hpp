#pragma once

#include <optional>

#include "box2d/box2d.h"

namespace game2d {

// Bodies are built using the following steps:
// Define a body with position, damping, etc.
// Use the world object to create the body.
// Define fixtures with a shape, friction, density, etc.
// Create fixtures on the body.

struct ActorComponent
{
  b2Body* body;
};

} // namespace game2d