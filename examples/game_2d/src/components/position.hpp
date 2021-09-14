#pragma once

namespace game2d {

struct Position
{};

struct PositionInt : public Position
{
  int x, y = 0;

  PositionInt() = default;
  PositionInt(int x, int y)
    : x(x)
    , y(y){};
};

struct PositionFloat : public Position
{
  float x, y = 0;

  PositionFloat() = default;
  PositionFloat(float x, float y)
    : x(x)
    , y(y){};
};

} // namespace game2d