// your header
#include "process_actor_actor_collisions.hpp"

// game2d
#include "physics/components.hpp"
#include "physics/helpers.hpp"

// other lib headers
#include <algorithm>
#include <glm/glm.hpp>

#include <ranges>
#include <vector>

namespace game2d {

void
update_actor_actor_collisions_system(entt::registry& r, SINGLETON_PhysicsComponent& p)
{
  // 1. get all the actors
  // 2. generate all possible collisions
  // 3. generate collision enter, exit, stay

  // generate filtered broadphase collisions
  //
  generate_filtered_broadphase_collisions(r, p.frame_collisions);

  // add actor-solid collisions
  // std::move(p.frame_solid_collisions.begin(), p.frame_solid_collisions.end(), std::back_inserter(p.frame_collisions));

  //  filter with narrowphase collisions
  //
  const auto colls = std::move(p.frame_collisions);
  for (const auto& coll : colls) {
    const auto a = static_cast<entt::entity>(coll.ent_id_0);
    const auto b = static_cast<entt::entity>(coll.ent_id_1);

    auto* a_circle = r.try_get<CircleCollider>(a);
    auto* b_circle = r.try_get<CircleCollider>(b);
    if (!a_circle || !b_circle) {
      p.frame_collisions.push_back(coll);
      continue;
    }

    // moving center probably shouldnt be here
    const auto& a_aabb = r.get<AABB>(a);
    const auto& b_aabb = r.get<AABB>(b);
    a_circle->center = a_aabb.center;
    b_circle->center = b_aabb.center;
    if (collide(*a_circle, *b_circle))
      p.frame_collisions.push_back(coll);
  }

  // std::vector<Collision2D> narrowphase_collisions;
  // generate_narrowphase_collisons(r, broadphase_collisions, narrowphase_collisions)

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
    for (auto& persistent_collision : p.persistent_collisions)
      persistent_collision.dirty = true;

    // Check the new collisions
    for (const auto& coll : p.frame_collisions) {
      const auto& pmap = p.persistent_collisions;

      const auto l = [&coll](const Collision2D& new_col) {
        return new_col.ent_id_0 == coll.ent_id_0 && new_col.ent_id_1 == coll.ent_id_1;
      };
      const auto result = std::find_if(pmap.begin(), pmap.end(), l);

      if (result == pmap.end()) {
        // New collision
        // fmt::println("ents: new coll" << "\n";
        p.collision_enter.push_back(coll);
        p.collision_stay.push_back(coll);
      } else {
        // Update collision
        p.collision_stay.push_back(coll);
      }

      // Set as new persistent collision with dirty: false
      p.persistent_collisions.push_back(coll);
    }

    // Check stale collisions
    const auto is_dirty = [](const Collision2D& coll) { return coll.dirty; };
    const auto [first, last] = std::ranges::remove_if(p.persistent_collisions, is_dirty);
    p.collision_exit.insert(p.collision_exit.end(), first, last);
    p.persistent_collisions.erase(first, last);
  }
};

} // namespace game2d