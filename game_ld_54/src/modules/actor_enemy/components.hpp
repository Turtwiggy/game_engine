#pragma once

#include <glm/glm.hpp>

namespace game2d {

struct EnemyComponent
{
  bool has_target = false;
  glm::ivec3 target;
};

} // namespace game2d