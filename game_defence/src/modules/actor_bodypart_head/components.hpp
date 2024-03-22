#pragma once

#include <entt/entt.hpp>

namespace game2d {

struct HeadComponent
{
  float time = 0.0f;

  float head_size_x = 5;
  float head_size_y = 10;

  float offset_y = 3.5f;
  float headbob_speed = 8.0f;
  float headbob_amplitude = 1.0f;
};

} // namespace game2d