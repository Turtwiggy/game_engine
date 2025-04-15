#pragma once

#include <glm/fwd.hpp>

namespace game2d {

enum class ChargerEnemyState
{
  START_APPROACH,
  APPROACHING,
  START_CHARGE_UP,
  CHARGE_UP,
  START_CHARGE,
  CHARGE,
  START_CATCH_YOUR_BREATH,
  CATCH_YOUR_BREATH
};

struct ChargerEnemyComponent
{
  ChargerEnemyState state = ChargerEnemyState::START_APPROACH;

  // APPROACHING
  float d2_to_start_charge = 400 * 400; // squared cuz distance2 check
  float approach_speed = 1.0f;

  // START_CHARGE_UP
  float time_to_charge_up = 2.0f;

  // START_CHARGE
  glm::vec2 start_charge_position;
  float charge_speed = 7.0f;

  // CHARGE
  float d2_to_charge = 450 * 450; // squared cuz distance2 check

  // START_CATCH_YOUR_BREATH
  float time_to_catch_breath = 5.0f;
};

} // namespace game2d