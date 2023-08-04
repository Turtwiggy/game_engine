#pragma once

#include <box2d/box2d.h>
#include <glm/glm.hpp>

namespace game2d {

struct ActorComponent
{
  b2Body* body;
  glm::ivec2 size;
};

} // namespace game2d