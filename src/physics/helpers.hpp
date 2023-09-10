#pragma once

#include "components.hpp"

// other lib headers
#include <glm/glm.hpp>

// other lib headers
#include <entt/entt.hpp>

// c++ lib headers
#include <vector>

namespace game2d {

bool
collide(const AABB& one, const AABB& two);

// Checks collisions between actor-actors.
void
generate_filtered_broadphase_collisions(entt::registry& r, std::vector<Collision2D>& results);

}; // namespace game2d
