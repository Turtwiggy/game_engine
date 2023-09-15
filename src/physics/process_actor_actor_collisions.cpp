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

void
game2d::update_actor_actor_collisions_system(entt::registry& r, SINGLETON_PhysicsComponent& p)
{
  // 1. get all the actors
  // 2. generate all possible collisions
  // 3. generate collision enter, exit, stay

  p.frame_collisions.clear();

  //
  // generate filtered broadphase collisions
  //

  generate_filtered_broadphase_collisions(r, p.frame_collisions);

  // TODO: filter with narrowphase collisions
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
      auto result = std::find_if(pmap.begin(), pmap.end(), [&coll](const Collision2D& new_col) {
        return new_col.ent_id_0 == coll.ent_id_0 && new_col.ent_id_1 == coll.ent_id_1;
      });
      if (result == pmap.end()) {
        // New collision
        // std::cout << "ents: new coll" << "\n";
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