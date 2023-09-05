// your header
#include "helpers.hpp"

// engine headers
#include "entt/helpers.hpp"
#include "maths/maths.hpp"

#include <algorithm>

namespace game2d {

bool
collide(const AABB& one, const AABB& two)
{
  const glm::ivec2 one_tl_pos = { one.center.x - one.size.x / 2, one.center.y - one.size.y / 2 };
  const glm::ivec2 two_tl_pos = { two.center.x - two.size.x / 2, two.center.y - two.size.y / 2 };

  // collision x-axis?
  bool collision_x = one_tl_pos.x + one.size.x > two_tl_pos.x && two_tl_pos.x + two.size.x > one_tl_pos.x;

  // collision y-axis?
  bool collision_y = one_tl_pos.y + one.size.y > two_tl_pos.y && two_tl_pos.y + two.size.y > one_tl_pos.y;

  return collision_x && collision_y;
};

void
generate_broadphase_collisions_xy(entt::registry& r, std::vector<Collision2D>& collisions)
{
  // store results of sorted aabb
  auto& physics = get_first_component<SINGLETON_PhysicsComponent>(r);
  physics.sorted_x.clear();
  physics.sorted_y.clear();

  // sort physics
  r.sort<PhysicsTransformXComponent>([](const auto& a, const auto& b) { return a.l < b.l; });
  r.sort<PhysicsTransformYComponent>([](const auto& a, const auto& b) { return a.t < b.t; });

  auto view = r.view<const PhysicsTransformXComponent, const PhysicsTransformYComponent, const AABB>();

  std::vector<entt::entity> active_list;
  active_list.reserve(view.size_hint());

  active_list.clear();
  view.use<const PhysicsTransformXComponent>();
  for (int i = 0; const auto& [new_obj, new_x, new_y, new_aabb] : view.each()) {
    physics.sorted_x.push_back(new_obj);

    // begin on the left of sorted_aabb.
    // add the first item from sorted_aabb to active_list.
    if (i == 0) {
      active_list.push_back(new_obj);
      ++i;
      continue;
    }

    // have a look at the next item in axis_list,
    // compare it with all the items currently in active_list. (currently just 1)
    auto it_1 = active_list.begin();
    while (it_1 != active_list.end()) {
      const auto& old_obj = *it_1;
      const auto& [old_x, old_y, old_aabb] = view.get(old_obj);

      // if the new item's left is > than the active_item's right
      if (new_x.l >= old_x.r) {
        // no possible collision!
        // remove the active_list item from the active list
        it_1 = active_list.erase(it_1);
      } else {
        // possible collision!
        // between new axis_list item and the current active_list item

        // Just do the AABB collision check
        if (collide(new_aabb, old_aabb)) {
          Collision2D coll;
          const auto id_0 = static_cast<uint32_t>(old_obj);
          const auto id_1 = static_cast<uint32_t>(new_obj);
          coll.ent_id_0 = glm::min(id_0, id_1);
          coll.ent_id_1 = glm::max(id_0, id_1);
          collisions.push_back(coll);
        }

        ++it_1;
      }
    }

    // Add the new item itself to active_list and continue with the next item in axis_list
    active_list.push_back(new_obj);
  }

  active_list.clear();
  view.use<const PhysicsTransformYComponent>();
  for (int i = 0; const auto& [new_obj, new_x, new_y, new_aabb] : view.each()) {
    physics.sorted_y.push_back(new_obj);

    // begin on the left of sorted_aabb.
    // add the first item from sorted_aabb to active_list.
    if (i == 0) {
      active_list.push_back(new_obj);
      ++i;
      continue;
    }

    // have a look at the next item in axis_list,
    // compare it with all the items currently in active_list. (currently just 1)
    auto it_1 = active_list.begin();
    while (it_1 != active_list.end()) {
      const auto& old_obj = *it_1;
      const auto& [old_x, old_y, old_aabb] = view.get(old_obj);

      // if the new item's top is > than the active_item's bottom
      if (new_y.b >= old_y.t) {
        // no possible collision!
        // remove the active_list item from the active list
        it_1 = active_list.erase(it_1);
      } else {
        // possible collision!
        // between new axis_list item and the current active_list item

        // Just do the AABB collision check
        if (collide(new_aabb, old_aabb)) {
          const auto id_0 = static_cast<uint32_t>(old_obj);
          const auto id_1 = static_cast<uint32_t>(new_obj);
          Collision2D coll;
          coll.ent_id_0 = glm::min(id_0, id_1);
          coll.ent_id_1 = glm::max(id_0, id_1);
          collisions.push_back(coll);
        }

        ++it_1;
      }
    }

    // Add the new item itself to active_list and continue with the next item in axis_list
    active_list.push_back(new_obj);
  }
};

void
generate_filtered_broadphase_collisions(entt::registry& r, std::vector<Collision2D>& collisions)
{
  // Do broad-phase check.
  generate_broadphase_collisions_xy(r, collisions);

  // remove duplicates
  collisions.erase(std::unique(collisions.begin(), collisions.end()), collisions.end());
};

}; // namespace game2d