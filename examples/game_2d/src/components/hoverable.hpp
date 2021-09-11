#pragma once

#include "components/colour.hpp"

namespace game2d {

struct Hoverable
{
  bool mouse_is_hovering = false;
  Colour hover_colour;
};

} // namespace game2d