#pragma once

#include <glm/glm.hpp>

namespace game2d {

struct DDAInput
{
  glm::vec2 start{ 0, 0 };
  glm::vec2 end{ 0, 0 };

  glm::vec2 map_size{ 30, 30 };
};

struct DDAOutput
{
  float distance = 0.0f;
  bool has_intersection = false;
  glm::vec2 intersection{ 0, 0 };
};

} // namespace game2d