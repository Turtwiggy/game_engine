#pragma once

#include <entt/fwd.hpp>

namespace game2d {

struct SINGLE_GoldComponent
{
  int amount = 0;
  int temp_amount_pickup = 0;  // collected during run
  int temp_amount_enemies = 0; // X enemies = 1 gold
};

} // namespace game2d.