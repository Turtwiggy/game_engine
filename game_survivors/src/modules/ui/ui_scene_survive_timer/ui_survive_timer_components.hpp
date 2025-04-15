#pragma once

#include <entt/fwd.hpp>

namespace game2d {

struct SurviveTimerComponent
{
  float time_left_max = 10 * 60;
  float time_left_cur = 10 * 60;
};

} // namespace game2d