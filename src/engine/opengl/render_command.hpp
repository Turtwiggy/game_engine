#pragma once

#include "engine/colour/colour.hpp"

namespace engine {

class RenderCommand
{
public:
  static void set_viewport(int x, int y, int width, int height);
  static void set_clear_colour(const SRGBColour& colour);
  static void clear();
};

} // namespace engine
