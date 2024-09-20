#pragma once

#include "engine/colour/colour.hpp"
#include <entt/entt.hpp>

#include <string>

namespace game2d {

struct SpaceLabelComponent
{
  // placeholder string that is deliberately fairly long
  std::string text = "PLACEHOLDER";
  engine::SRGBColour ui_colour{ 1.0f, 1.0f, 1.0f, 1.0f };
};

} // namespace game2d