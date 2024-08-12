#pragma once

#include <entt/entt.hpp>

#include <functional>

namespace game2d {

enum class DISTANCE_CHECK_TYPE
{
  LESS_THAN_OR_EQUAL,
  GREATER_THAN_OR_EQUAL,
};

struct DistanceCheckComponent
{
  // could either be an entity with positions.. or raw positions.
  // for the moment, use entities
  entt::entity e0;
  entt::entity e1;

  int d2 = INT_MIN;
  std::function<void(entt::registry&, DistanceCheckComponent&)> action;

  // when to call action()
  DISTANCE_CHECK_TYPE type = DISTANCE_CHECK_TYPE::LESS_THAN_OR_EQUAL;
};

} // namespace game2d