// your header
#include "systems/destroy_on_collide.hpp"

// components
#include "modules/physics/components.hpp"
#include "modules/renderer/components.hpp"

// other lib headers
#include <imgui.h>

// Move an Actor
void
game2d::update_destroy_on_collide_system(entt::registry& registry, engine::Application& app, float dt)
{
  SINGLETON_PhysicsComponent& p = registry.ctx<SINGLETON_PhysicsComponent>();

  const auto& view = registry.view<const CollidableComponent>();
  view.each([&registry, &p](const auto entity, const auto& col) {
    for (const auto& coll : p.collision_enter) {
      uint32_t ent_id = static_cast<uint32_t>(entity);
      bool destroy = coll.ent_id_0 == ent_id || coll.ent_id_1 == ent_id;
      bool valid_destroy = registry.valid(entity);
      if (destroy && valid_destroy) {
        registry.destroy(entity);
      }
    }
  });
};