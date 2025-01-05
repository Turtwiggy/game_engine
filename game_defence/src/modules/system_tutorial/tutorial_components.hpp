#pragma once

#include <entt/entt.hpp>

namespace game2d {

struct SINGLE_TutorialMetrics
{
  int turns_taken = 0;
  int max_turns = 1;
  std::string objective;
  int expected_enemies = 0;
};

} // namespace game2d