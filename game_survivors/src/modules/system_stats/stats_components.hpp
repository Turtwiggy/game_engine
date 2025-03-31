#pragma once

#include <entt/fwd.hpp>

namespace game2d {

struct SINGLE_SurviveStatsComponent
{
  int enemies_killed = 0;
  int gold_earned = 0;
};

} // namespace game2d