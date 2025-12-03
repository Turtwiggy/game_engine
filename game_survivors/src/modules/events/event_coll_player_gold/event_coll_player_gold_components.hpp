#pragma once

#include <entt/fwd.hpp>

namespace game2d {

struct ItemGoldComponent
{
  int amount = 1;
};

struct WantToGetGoldEvent
{
  int amount = 1;
};

} // namespace game2d