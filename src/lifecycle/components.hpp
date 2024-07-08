#pragma once

#include "entt/entt.hpp"

#include <unordered_set>

namespace game2d {

// This idea could probably be expanded to:
// After a certain amount of time (i.e. a cooldown)
// execute some behaviour in the next fixed tick possible.

struct HasParentComponent
{
  entt::entity parent = entt::null;
};

struct EntityTimedLifecycle
{
  int milliseconds_alive_max = 3 * 1000; // seconds
  int milliseconds_alive = 0;            // counts from 0 up to _max
};

struct SINGLETON_EntityBinComponent
{
  std::unordered_set<entt::entity> dead;      // to destroy next fixed update
  std::vector<entt::entity> things_that_died; // destroyed last fixed update
};

// added to an entity when made via create_gameplay()
// removed at the next FixedUpdate()
struct WaitForInitComponent
{
  bool placeholder = true;
};

};