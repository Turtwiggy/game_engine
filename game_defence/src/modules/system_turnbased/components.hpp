#pragma once

#include <entt/entt.hpp>

namespace game2d {

struct TurnBasedUnitComponent
{
  bool placeholder = true;
};

struct MoveLimitComponent
{
  int amount = 1;
};

} // namespace game2d