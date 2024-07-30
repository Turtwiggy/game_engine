#pragma once

#include <entt/entt.hpp>

#include <functional>

namespace game2d {

struct DistanceCheckComponent
{
  // could either be an entity with positions.. or raw positions.
  // for the moment, use entities
  entt::entity e0;
  entt::entity e1;

  int d2 = INT_MIN;
  std::function<void(entt::registry&)> action;
};

} // namespace game2d