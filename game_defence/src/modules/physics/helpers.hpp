#pragma once

#include "components.hpp"
#include "renderer/components.hpp"

// other lib headers
#include <glm/glm.hpp>

// other lib headers
#include <entt/entt.hpp>

// c++ lib headers
#include <vector>

namespace game2d {

[[nodiscard]] std::vector<Collision2D>
do_move_x(entt::registry& r, const entt::entity& e, AABB& aabb, VelocityComponent& vel);

[[nodiscard]] std::vector<Collision2D>
do_move_y(entt::registry& r, const entt::entity& e, AABB& aabb, VelocityComponent& vel);

// Checks collisions between actor-actors.
void
generate_filtered_broadphase_collisions(entt::registry& r, std::vector<Collision2D>& results);

}; // namespace game2d
