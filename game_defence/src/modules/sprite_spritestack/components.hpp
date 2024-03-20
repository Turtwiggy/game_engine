#pragma once

#include <glm/glm.hpp>

namespace game2d {

struct SpritestackComponent
{
  int spritestack_index = 0;
  SpritestackComponent(int index)
    : spritestack_index(index){};
};

struct RotateOnSpotComponent
{
  bool placeholder = true;
};

} // namespace game2d