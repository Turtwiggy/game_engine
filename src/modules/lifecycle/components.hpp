#pragma once

#include <entt/entt.hpp>

#include <unordered_set>

namespace game2d {

struct EntityTimedLifecycle
{
  float time_alive_max = 10.0f;
  float time_alive = 0.0f;
};

// SINGLETONS

struct SINGLETON_EntityBinComponent
{
  std::unordered_set<entt::entity> dead;
};

};