#pragma once

#include <entt/entt.hpp>

namespace game2d {

struct ParticleComponent
{
  bool placeholder = true;
};

struct ScaleOverTimeComponent
{
  float timer = 0.0f;
  float seconds_until_complete = 3.0f;
  float start_size = 16.0f;
  float end_size = 0.0f;
};

} // namespace game2d