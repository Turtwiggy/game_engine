#pragma once

#include "engine/colour.hpp"

#include <glm/glm.hpp>

namespace game2d {

struct SelectableComponent
{
  bool is_selected = false;
};

struct HighlightComponent
{
  engine::SRGBColour start_colour = { 255, 255, 255, 1.0f };
  engine::SRGBColour highlight_colour = { 255, 255, 255, 1.0f };
};

} // namespace game2d
