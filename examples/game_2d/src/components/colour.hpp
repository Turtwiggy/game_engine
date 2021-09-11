#pragma once

#include <glm/glm.hpp>

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
};

} // namespace game2d