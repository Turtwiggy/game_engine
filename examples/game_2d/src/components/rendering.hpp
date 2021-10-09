#pragma once

#include "helpers/spritemap.hpp"

namespace game2d {

struct Colour
{
  glm::vec4 colour = { 1.0f, 1.0f, 1.0f, 1.0f };

  Colour() = default;
  Colour(float r, float g, float b, float a)
  {
    colour.x = r;
    colour.y = g;
    colour.z = b;
    colour.a = a;
  };
  Colour(const Colour& c)
  {
    colour.x = c.colour.x;
    colour.y = c.colour.y;
    colour.z = c.colour.z;
    colour.a = c.colour.a;
  };
  Colour(const glm::vec4& c)
  {
    colour.x = c.x;
    colour.y = c.y;
    colour.z = c.z;
    colour.a = c.a;
  };
};

struct PositionInt
{
  int x, y = 0;

  PositionInt() = default;
  PositionInt(int x, int y)
    : x(x)
    , y(y){};
};

struct PositionFloat
{
  float x, y = 0;

  PositionFloat() = default;
  PositionFloat(float x, float y)
    : x(x)
    , y(y){};
};

struct Size
{
  int w, h;

  Size() = default;
  Size(int w, int h)
    : w(w)
    , h(h){};
};

struct Sprite
{
  sprite::type sprite = sprite::type::SQUARE;

  Sprite() = default;
  Sprite(sprite::type sprite)
    : sprite(sprite){};
};

struct ZIndex
{
  int index = 0;

  ZIndex() = default;
  ZIndex(int index)
    : index(index){};
};

} // namespace game2d