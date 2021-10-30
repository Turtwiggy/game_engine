#pragma once

// game headers
#include "helpers/spritemap.hpp"

namespace game2d {

struct ColourComponent
{
  glm::vec4 colour = { 1.0f, 1.0f, 1.0f, 1.0f };

  ColourComponent() = default;
  ColourComponent(float r, float g, float b, float a)
  {
    colour.x = r;
    colour.y = g;
    colour.z = b;
    colour.a = a;
  };
  ColourComponent(const ColourComponent& c)
  {
    colour.x = c.colour.x;
    colour.y = c.colour.y;
    colour.z = c.colour.z;
    colour.a = c.colour.a;
  };
  ColourComponent(const glm::vec4& c)
  {
    colour.x = c.x;
    colour.y = c.y;
    colour.z = c.z;
    colour.a = c.a;
  };
};

struct PositionIntComponent
{
  int x = 0;
  int y = 0;
  float dx = 0.0f; // remainders in the x and y dirs
  float dy = 0.0f;

  PositionIntComponent() = default;
  PositionIntComponent(int x, int y)
    : x(x)
    , y(y){};
};

struct PositionFloat
{
  float x = 0.0f;
  float y = 0.0f;

  PositionFloat() = default;
  PositionFloat(float x, float y)
    : x(x)
    , y(y){};
};

struct SizeComponent
{
  int w = 0;
  int h = 0;

  SizeComponent() = default;
  SizeComponent(int w, int h)
    : w(w)
    , h(h){};
};

struct SpriteComponent
{
  sprite::type sprite = sprite::type::EMPTY;

  SpriteComponent() = default;
  SpriteComponent(sprite::type sprite)
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