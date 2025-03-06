#pragma once

#include <entt/entt.hpp>

namespace game2d {

struct SurviveTimerComponent
{
  float time_left_max = 20 * 60;
  float time_left_cur = 20 * 60;
};

} // namespace game2d