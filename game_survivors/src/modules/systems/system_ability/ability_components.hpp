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

  // the players input sets these to true
  bool ability_1_flash_when_off_cooldown = false;
  bool ability_2_flash_when_off_cooldown = false;

  // ui display
  bool ability_1_pressed = true; // if set to true, dont require them to press it once
  bool ability_2_pressed = true; // if set to true, dont require them to press it once
  float ability_1_alpha = 1.0f;
  float ability_2_alpha = 1.0f;
};

} // namespace game2d