#pragma once

#include "components.hpp"

#include <entt/entt.hpp>

namespace game2d {

void
emplace_or_replace_physics_world(entt::registry& r);

void
create_physics_actor(entt::registry& r, const entt::entity e, const PhysicsDescription& desc);

} // namespace game2d