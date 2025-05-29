#pragma once

#include <entt/fwd.hpp>

namespace game2d {

struct AoE_SlowComponent
{
  float time_between_slow_seconds = 0.0f;
  float time_between_slow_seconds_max = 0.5f;
};

} // namespace game2d