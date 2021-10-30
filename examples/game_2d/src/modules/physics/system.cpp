// your header
#include "system.hpp"

// game2d
#include "modules/physics/components.hpp"
#include "modules/physics/helpers.hpp"
#include "modules/renderer/components.hpp"

// other lib headers
#include <glm/glm.hpp>

void
game2d::init_physics_system(entt::registry& registry)
{
  registry.set<SINGLETON_PhysicsComponent>(SINGLETON_PhysicsComponent());
};

void
game2d::update_physics_system(entt::registry& registry, engine::Application& app, float dt)
{
  SINGLETON_PhysicsComponent& p = registry.ctx<SINGLETON_PhysicsComponent>();

  // ImGui::Begin("Physics Debug");
  {
    p.collidable.clear();
    const auto& view = registry.view<const CollidableComponent, const PositionIntComponent, const SizeComponent>();
    view.each([&registry, &p](const auto entity, const auto& col, const auto& pos, const auto& size) {
      auto& tag_comp = registry.get<TagComponent>(entity);

      PhysicsObject po;
      po.ent_id = static_cast<uint32_t>(entity);
      po.x_tl = static_cast<int>(pos.x - size.w / 2.0f);
      po.y_tl = static_cast<int>(pos.y - size.h / 2.0f);
      po.w = size.w;
      po.h = size.h;

      p.collidable.push_back(po);
      // ImGui::Text("id:%i Name:%s Layer:%i", po.ent_id, tag_comp.tag.c_str(), po.ent_id);
    });
  }

  generate_filtered_broadphase_collisions(p.collidable, p.filtered_collisions);
  // ImGui::End();
};