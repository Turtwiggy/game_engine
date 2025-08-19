#pragma once

#include <entt/fwd.hpp>

namespace game2d {

struct UiCursorComponent
{
  float cursor_size = 10;
  glm::vec2 cursor_offset_base = glm::vec2{ cursor_size, -12 };
  glm::vec2 cursor_offset = glm::vec2{ 0, 0 };
  float cursor_wiggle_time = 0.0f;
  float cursor_wiggle = 0.0f;
  float cursor_wiggle_frequency = 15.0f;
  float cursor_wiggle_amplitude = 2.0f;
};

} // namespace game2d