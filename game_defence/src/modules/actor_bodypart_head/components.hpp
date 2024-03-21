#pragma once

#include <entt/entt.hpp>

namespace game2d {

struct HeadComponent
{
  float time = 0.0f;

  float head_size_x = 8;
  float head_size_y = 11;

  float offset_y = 5.0f;
  float headbob_speed = 4.0f;
  float headbob_amplitude = 2.5f;
};

} // namespace game2d