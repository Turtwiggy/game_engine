#pragma once

#include "actors.hpp"

#include "glm/glm.hpp"
#include <entt/entt.hpp>
#include <unordered_set>

namespace game2d {

// This idea could probably be expanded to:
// After a certain amount of time (i.e. a cooldown)
// execute some behaviour in the next fixed tick possible.

struct EntityTimedLifecycle
{
  int milliseconds_alive_max = 5 * 1000; // seconds
  int milliseconds_alive = 0;
};

struct SINGLETON_EntityBinComponent
{
  std::unordered_set<entt::entity> dead;
};

struct CreateEntityRequest
{
  EntityType entity_type;
  glm::ivec3 position;
  glm::ivec3 velocity;
};

};