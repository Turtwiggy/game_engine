#pragma once

#include "actors.hpp"

#include "glm/glm.hpp"
#include <entt/entt.hpp>
#include <optional>
#include <string>
#include <unordered_set>
#include <vector>

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

  std::vector<entt::entity> created_this_frame;
};

struct CreateEntityRequest
{
  EntityType type = EntityType::empty;
  glm::ivec3 position{ 0, 0, 0 };
  glm::vec3 velocity{ 0, 0, 0 };
  std::optional<std::string> sprite = std::nullopt;
  entt::entity parent = entt::null;
};

struct HasParentComponent
{
  entt::entity parent = entt::null;
};

};