#pragma once

#include "colour/colour.hpp"

namespace game2d {

struct SelectableComponent
{
  bool is_selected = true;
};

struct HoverComponent
{
  engine::SRGBColour regular_colour = { 255, 255, 255, 1.0f };
  engine::SRGBColour hover_colour = { 255, 255, 255, 1.0f };
};

} // namespace game2d