#pragma once

#include <entt/entt.hpp>

namespace game2d {

// use box2d to get nearest target
entt::entity
query_aabb_get_nearest_target(entt::registry& r, const entt::entity e);

} // namespace game2d