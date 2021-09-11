#pragma once

#include "helpers/spritemap.hpp"

namespace game2d {

struct ZIndex
{
  int index = 0;

  ZIndex() = default;
  ZIndex(int index)
    : index(index){};
};

} // namespace game2d