#pragma once

#include <glm/glm.hpp>

namespace game2d {

struct TransformComponent
{
  glm::vec3 position{ 0, 0, 0 };
  glm::vec3 rotation_radians = { 0, 0, 0 };
  glm::vec3 scale{ 0, 0, 0 };

  int z_index = 0;
};

} // namespace game2d