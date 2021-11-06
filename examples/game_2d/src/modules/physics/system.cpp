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
  // 1. get all the collidable components
  // 2. generate all possible collisions
  // 3. generate collision enter, exit, stay events between collision events

  SINGLETON_PhysicsComponent& p = registry.ctx<SINGLETON_PhysicsComponent>();

  {
    p.collidable.clear();
    PhysicsObject po;
    const auto& view = registry.view<const CollidableComponent, const PositionIntComponent, const SizeComponent>();
    view.each([&registry, &po](const auto entity, const auto& col, const auto& pos, const auto& size) {
      po.ent_id = static_cast<uint32_t>(entity);
      po.x_tl = static_cast<int>(pos.x - size.w / 2.0f);
      po.y_tl = static_cast<int>(pos.y - size.h / 2.0f);
      po.w = size.w;
      po.h = size.h;

      p.collidable.push_back(po);
    });
  }

  generate_filtered_broadphase_collisions(p.collidable, p.frame_collisions);

  {
    // There's basically 3 states needed to capture:
    // OnCollisionEnter
    // OnCollisionStay
    // OnCollisionExit
    // p.frame_collisions contains enter and stay collisions
    // p.persistent_collisions contains stay and exit collision
    p.collision_enter.clear();
    p.collision_stay.clear();
    p.collision_exit.clear();

    // Set any persistent collisions to dirty, as they're from last frame
    for (auto& persistent_collision : p.persistent_collisions) {
      persistent_collision.second.dirty = true;
    }

    for (const auto& frame_collision : p.frame_collisions) {
      const auto& val = frame_collision.first;
      const auto& pmap = p.persistent_collisions;
      auto result = std::find_if(pmap.begin(), pmap.end(), [val](const auto& col) { return col.first == val; });
      if (result == pmap.end()) {
        // New collision
        // std::cout << "ents: new coll" << std::endl;
        p.collision_enter.push_back(frame_collision.second);
        p.collision_stay.push_back(frame_collision.second);
      } else {
        // Update collision
        p.collision_stay.push_back(frame_collision.second);
      }
      p.persistent_collisions[frame_collision.first] = frame_collision.second;
      p.persistent_collisions[frame_collision.first].dirty = false;
    }

    // Check stale collisions
    std::map<uint64_t, Collision2D>::iterator it;
    for (it = p.persistent_collisions.begin(); it != p.persistent_collisions.end();) {
      if (!it->second.dirty) {
        ++it;
        continue;
      }
      // std::cout << "ents: exit coll" << std::endl;
      p.collision_exit.push_back(it->second);
      p.persistent_collisions.erase(it++);
    }
  }

  // ImGui::End();
};