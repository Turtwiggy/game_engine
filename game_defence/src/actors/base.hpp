#pragma once

#include "bags/core.hpp"

#include <entt/entt.hpp>

namespace game2d {

/*
  In a pure ECS design, entities are often just IDs with no intrinsic meaning;
  their behavior and identity come entirely from the components attached to them.
*/

class Factory_BaseActor
{
public:
  static entt::entity create(entt::registry& r, const EntityData& desc);
};

} // namespace game2d