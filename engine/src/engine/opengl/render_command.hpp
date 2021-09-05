#pragma once

// other library headers
#include <glm/glm.hpp>

namespace engine {

class RenderCommand
{
public:
  static void init();

  static void set_viewport(int x, int y, int width, int height);

  static void set_clear_colour(const glm::vec4& color);

  static void set_depth_testing(bool toggle);

  static void clear();
};

} // namespace engine
