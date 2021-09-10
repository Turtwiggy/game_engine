#pragma once

#include "components/colour.hpp"

namespace game2d {

struct Hoverable
{
  bool hovering = false;
  Colour normal_colour;
  Colour hover_colour;
};

} // namespace game2d