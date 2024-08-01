#pragma once

#include "components.hpp"

#include <entt/entt.hpp>

namespace game2d {

void
create_physics_actor(entt::registry& r, const entt::entity e, const PhysicsDescription& desc);

} // namespace game2d