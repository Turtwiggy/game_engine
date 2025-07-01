#pragma once

#include <entt/fwd.hpp>

namespace game2d {

struct IslanderAiComponent
{
  float time_between_ai_checks_max = 1.0f;
  float time_between_ai_checks_cur = 1.0f;
};

} // namespace game2d