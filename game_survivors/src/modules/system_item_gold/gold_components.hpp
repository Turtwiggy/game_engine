#pragma once

#include <entt/entt.hpp>

namespace game2d {

struct SINGLE_GoldComponent
{
  int amount = 0;
  int temp_amount = 0; // collected during run
};

} // namespace game2d.