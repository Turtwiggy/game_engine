#pragma once

#include "helpers/spritemap.hpp"

namespace game2d {

struct Sprite
{
  sprite::type sprite = sprite::type::SQUARE;

  Sprite() = default;
  Sprite(sprite::type sprite)
    : sprite(sprite){};
};

} // namespace game2d