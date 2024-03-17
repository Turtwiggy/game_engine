#pragma once

#include <glm/glm.hpp>

namespace game2d {

struct SpritestackComponent
{
  glm::ivec2 base_position{ 0, 0 };
  int spritestack_position = 0;
  int layer_height = 5;
};

struct RotateOnSpotComponent
{
  bool placeholder = true;
};

} // namespace game2d