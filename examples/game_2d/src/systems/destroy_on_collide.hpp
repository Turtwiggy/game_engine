#pragma once

// engine headers
#include "engine/application.hpp"

// other lib headers
#include <entt/entt.hpp>

namespace game2d {

void
update_destroy_on_collide_system(entt::registry& registry, engine::Application& app, float dt)
{
  //
  SINGLETON_PhysicsComponent& p = registry.ctx<SINGLETON_PhysicsComponent>();

  const auto& view = registry.view<const CollidableComponent>();
  view.each([&registry, &p](const auto entity, const auto& col) {
    //
    //
    for (const auto& coll : p.collision_enter) {
      uint32_t ent_id = static_cast<uint32_t>(entity);
      bool destroy = coll.ent_id_0 == ent_id || coll.ent_id_1 == ent_id;
      if (destroy) {
        registry.destroy(entity);
      }
    }
  });
}

} // namespace game2d