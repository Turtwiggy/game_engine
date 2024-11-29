#pragma once

#include "components.hpp"

#include <entt/entt.hpp>

namespace game2d {

void
emplace_or_replace_physics_world(entt::registry& r);

void
create_physics_actor(entt::registry& r, const entt::entity e, const PhysicsDescription& desc);

inline void
create_physics_actor_static(entt::registry& r, const entt::entity e, const glm::ivec2& position, const glm::ivec2& size)
{
  PhysicsDescription desc;
  desc.type = b2_staticBody;
  desc.is_bullet = false;
  desc.density = 1.0;
  desc.position = position;
  desc.size = size;
  create_physics_actor(r, e, desc);
};

inline void
create_physics_actor_dynamic(entt::registry& r,
                             const entt::entity e,
                             const glm::ivec2& position,
                             const glm::ivec2& size,
                             bool is_bullet = false)
{
  PhysicsDescription desc;
  desc.type = b2_dynamicBody;
  desc.is_bullet = is_bullet;
  desc.density = 1.0;
  desc.position = position;
  desc.size = size;
  desc.is_sensor = is_bullet; // for the moment, bullets only sensor
  create_physics_actor(r, e, desc);
};

} // namespace game2d