#pragma once

#include <glm/glm.hpp>

namespace game2d {

struct TransformComponent
{
  glm::ivec3 position{ 0, 0, 0 };
  glm::vec3 rotation_radians = { 0, 0, 0 };
  glm::ivec3 scale{ 0, 0, 0 };
};

} // namespace game2d