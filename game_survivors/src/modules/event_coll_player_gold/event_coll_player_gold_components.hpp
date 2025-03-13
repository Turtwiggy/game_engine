#pragma once

#include <entt/fwd.hpp>

namespace game2d {

struct ItemGoldComponent
{
  bool placeholder = true;
};

struct WantToGetGoldEvent
{
  int amount = 1;
};

} // namespace game2d