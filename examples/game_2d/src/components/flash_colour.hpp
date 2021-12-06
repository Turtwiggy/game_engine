#pragma once

#include <glm/glm.hpp>

namespace game2d {

struct FlashColourComponent
{
  glm::vec4 start_colour = { 1.0f, 1.0f, 1.0f, 1.0f };
  glm::vec4 flash_colour = { 1.0f, 1.0f, 1.0f, 1.0f };
};

} // namespace game2d