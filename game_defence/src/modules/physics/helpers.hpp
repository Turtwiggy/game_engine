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

enum class CollisionAxis
{
  x,
  y
};

// [[nodiscard]] bool
// collide(const PhysicsTransformComponent& one, const PhysicsTransformComponent& two);

// [[nodiscard]] bool
// collides(const PhysicsTransformComponent& one, const std::vector<PhysicsTransformComponent>& others);

[[nodiscard]] std::vector<Collision2D>
do_move_x(entt::registry& r,
          entt::entity& entity,
          int amount,
          TransformComponent& transform,
          PhysicsTransformXComponent& x,
          const PhysicsTransformYComponent& y);
[[nodiscard]] std::vector<Collision2D>
do_move_y(entt::registry& r,
          entt::entity& entity,
          int amount,
          TransformComponent& transform,
          const PhysicsTransformXComponent& x,
          PhysicsTransformYComponent& y);

// Checks collisions between actor-actors.
void
generate_filtered_broadphase_collisions(entt::registry& r, std::vector<Collision2D>& results);

}; // namespace game2d
