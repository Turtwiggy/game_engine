#pragma once

#include <entt/fwd.hpp>

namespace game2d {

struct SurviveTimerComponent
{
  bool game_started = false;

  float time_left_max = 10 * 60;
  float time_left_cur = 10 * 60;

  int minute = 10;
  bool new_minute_hit = false;
};

} // namespace game2d