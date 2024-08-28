#pragma once

#include "colour/colour.hpp"

#include <glm/glm.hpp>

namespace game2d {

struct Light
{
  bool enabled = false;
  glm::vec2 pos;
  engine::SRGBColour colour;
  float luminence = 0.6f;
};

} // namespace game2d