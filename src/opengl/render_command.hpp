#pragma once

#include "colour/colour.hpp"

// other library headers
#include <glm/glm.hpp>

namespace engine {

class RenderCommand
{
public:
  static void set_viewport(int x, int y, int width, int height);
  static void set_clear_colour_srgb(const SRGBColour& colour);
  static void set_clear_colour_linear(const LinearColour& colour);
  static void clear();
};

} // namespace engine
