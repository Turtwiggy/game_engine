#pragma once

namespace game2d {

struct Position
{
  int x, y = 0;

  Position() = default;
  Position(int x, int y)
    : x(x)
    , y(y){};
};

} // namespace game2d