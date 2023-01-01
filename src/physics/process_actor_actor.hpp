#pragma once

#include "components.hpp"

#include <entt/entt.hpp>

namespace game2d {

void
update_actor_actor_system(entt::registry& registry, SINGLETON_PhysicsComponent& p);

}; // namespace game2d