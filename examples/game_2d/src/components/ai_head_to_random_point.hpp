#pragma once

#include <glm/glm.hpp>

namespace game2d {

struct AIHeadToRandomPoint
{
  bool destination_reached = false;
  glm::vec3 random_point;
};

} // namespace game2d