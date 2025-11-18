#pragma once

#include <entt/fwd.hpp>

namespace game2d {

struct AbilityComponent
{
  // ability 1: anchor
  float ability_1_cooldown = 3.5f;
  float ability_1_cooldown_left = 0.0f;
  bool ability_1_in_progress = false;

  // ability 2: boop
  float ability_2_cooldown = 6.5f;
  float ability_2_cooldown_left = 0.0f;
  bool ability_2_in_progress = false;
};

} // namespace game2d