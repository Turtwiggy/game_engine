#pragma once

#include "engine/lifecycle/components.hpp"

#include <entt/entt.hpp>

namespace game2d {

class EntityPool
{
public:
  std::vector<entt::entity> instances;

  void update(entt::registry& r, const size_t desired);
};

} // namespace game2d