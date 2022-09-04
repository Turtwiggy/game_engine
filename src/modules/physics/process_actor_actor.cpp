// your header
#include "process_actor_actor.hpp"

// game2d
#include "modules/physics/components.hpp"
#include "modules/physics/helpers.hpp"
#include "modules/renderer/components.hpp"

// other lib headers
#include <glm/glm.hpp>
#include <vector>

void
game2d::update_actor_actor_system(entt::registry& registry)
{
  // 1. get all the actors
  // 2. generate all possible collisions
  // 3. generate collision enter, exit, stay

  auto& p = registry.ctx().at<SINGLETON_PhysicsComponent>();

  std::vector<PhysicsObject> actors;
  {
    const auto& entt_actors =
      registry.view<const TransformComponent, const PhysicsSizeComponent, const PhysicsActorComponent>();
    PhysicsObject po;
    entt_actors.each([&actors, &po](const auto entity, const auto& transform, const auto& size, const auto& actor) {
      po.ent_id = static_cast<uint32_t>(entity);
      po.x_tl = static_cast<int>(transform.position.x - glm::abs(size.w) / 2.0f);
      po.y_tl = static_cast<int>(transform.position.y - glm::abs(size.h) / 2.0f);
      po.w = glm::abs(size.w);
      po.h = glm::abs(size.h);

      actors.push_back(po);
    });
  }

  generate_filtered_broadphase_collisions(actors, p.frame_collisions);

  {
    // There's 3 states needed to capture:
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
        // std::cout << "ents: new coll" << "\n";
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
      // std::cout << "ents: exit coll" << "\n";
      p.collision_exit.push_back(it->second);
      p.persistent_collisions.erase(it++);
    }
  }
};