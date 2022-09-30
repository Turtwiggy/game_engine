#pragma once

// components
#include "modules/physics/components.hpp"
#include "modules/renderer/components.hpp"

// other lib headers
#include <glm/glm.hpp>

// other lib headers
#include <entt/entt.hpp>

// c++ lib headers
#include <map>
#include <vector>

namespace game2d {

enum class CollisionAxis
{
  x,
  y
};

bool
collide(const PhysicsTransformComponent& one, const PhysicsTransformComponent& two);

bool
collides(const PhysicsTransformComponent& one, const std::vector<PhysicsTransformComponent>& others);

void
do_move(entt::registry& r,
        entt::entity& entity,
        int amount,
        TransformComponent& transform,
        const PhysicsTransformComponent& ptc,
        const CollisionAxis& axis);

// Checks collisions between actor-actors.
void
generate_filtered_broadphase_collisions(entt::registry& r, std::map<uint64_t, Collision2D>& collision_results);

}; // namespace game2d
