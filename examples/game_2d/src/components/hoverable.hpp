#pragma once

#include "components/colour.hpp"

namespace game2d {

struct Hoverable
{
  bool mouse_is_hovering = false;
  Colour hover_colour = Colour(1.0f, 0.0f, 0.0f, 1.0f);
};

} // namespace game2d