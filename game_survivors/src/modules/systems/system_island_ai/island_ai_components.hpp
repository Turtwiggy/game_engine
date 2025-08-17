#pragma once

#include <entt/fwd.hpp>

namespace game2d {

struct IslanderAiComponent
{
  const float time_between_ai_checks_min = 0.5f;
  const float time_between_ai_checks_max = 1.5f;
  float time_between_ai_checks_cur = time_between_ai_checks_max;
};

} // namespace game2d