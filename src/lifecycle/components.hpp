#pragma once

#include <entt/entt.hpp>

#include <unordered_set>

namespace game2d {

// This idea could probably be expanded to:
// After a certain amount of time (i.e. a cooldown)
// execute some behaviour in the next fixed tick possible.

struct EntityTimedLifecycle
{
  int milliseconds_alive_max = 10 * 1000;
  int milliseconds_alive = 0;
};

struct SINGLETON_EntityBinComponent
{
  std::unordered_set<entt::entity> dead;
};

};