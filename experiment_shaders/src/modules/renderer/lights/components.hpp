#pragma once

#include "engine/colour/colour.hpp"

#include <glm/glm.hpp>

namespace game2d {

struct Light
{
  bool enabled = false;
  glm::vec2 pos{ 0.0f, 0.0f };
  engine::SRGBColour colour;
  float luminence = 0.6f;
};

struct LightOccluderComponent
{
  bool placeholder = true;
};

struct LightEmitterComponent
{
  bool placeholder = true;
};

} // namespace game2d