#pragma once

#include <entt/fwd.hpp>

namespace game2d {

struct SINGLE_GoldComponent
{
  int amount = 0;
  int temp_amount_pickup = 0;  // collected during run
  int temp_amount_enemies = 0; // X enemies = 1 gold

  // gold bag should drop 1 gold every 50 enemies
  const int enemies_to_kill_before_next_drop_max = 50;
  int enemies_to_kill_before_next_drop_cur = 50;
};

} // namespace game2d.