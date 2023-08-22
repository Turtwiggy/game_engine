// your header
#include "helpers.hpp"

// engine headers
#include "entt/helpers.hpp"
#include "maths/maths.hpp"
#include "renderer/components.hpp"

#include <algorithm>

namespace game2d {

bool
collide(const AABB& one, const AABB& two)
{
  const glm::ivec2 one_tl_pos = { one.center.x - one.size.x / 2, one.center.y - one.size.y / 2 };
  const glm::ivec2 two_tl_pos = { two.center.x - two.size.x / 2, two.center.y - two.size.y / 2 };

  // collision x-axis?
  bool collision_x = one_tl_pos.x + one.size.x >= two_tl_pos.x && two_tl_pos.x + two.size.x >= one_tl_pos.x;

  // collision y-axis?
  bool collision_y = one_tl_pos.y + one.size.y >= two_tl_pos.y && two_tl_pos.y + two.size.y >= one_tl_pos.y;

  return collision_x && collision_y;
};

static float PPM = 16.0f;

std::vector<Collision2D>
do_move_x(entt::registry& r, const entt::entity& entity, AABB& aabb, VelocityComponent& vel)
{
  vel.remainder_x += (vel.x / PPM);
  int amount = static_cast<int>(vel.remainder_x);

  std::vector<Collision2D> collisions;
  if (amount == 0)
    return collisions;

  // this consumes it, so no frame jump
  vel.remainder_x -= amount;

  const int sign = glm::sign(amount);
  const auto& solids = r.view<const PhysicsSolidComponent, const AABB>();

  while (amount != 0) {

    // would-be updated position
    AABB updated_pos = aabb;
    updated_pos.center.x += sign;

    // Check if the updated position would collide with anything
    for (auto [o_entity, o_psolid, o_aabb] : solids.each()) {
      bool same = entity == o_entity;

      if (!same && collide(updated_pos, o_aabb)) {
        Collision2D collision;
        const auto id_0 = static_cast<uint32_t>(entity);
        const auto id_1 = static_cast<uint32_t>(o_entity);
        collision.ent_id_0 = glm::min(id_0, id_1);
        collision.ent_id_1 = glm::max(id_0, id_1);
        collisions.push_back(collision);
      }
    }
    if (collisions.size() > 0)
      return collisions;

    // Move player if empty space
    aabb.center.x += sign;
    amount -= sign;

    if (amount < 0 && sign > 0)
      std::cout << "never going to hit 0\n";
  }

  return collisions;
};

std::vector<Collision2D>
do_move_y(entt::registry& r, const entt::entity& entity, AABB& aabb, VelocityComponent& vel)
{
  vel.remainder_y += (vel.y / PPM);
  int amount = static_cast<int>(vel.remainder_y);

  std::vector<Collision2D> collisions;
  if (amount == 0)
    return collisions;

  // this consumes it, so no frame jump
  vel.remainder_y -= amount;

  const int sign = glm::sign(amount);
  const auto& solids = r.view<const PhysicsSolidComponent, const AABB>();

  while (amount != 0) {

    // would-be updated position
    AABB updated_pos = aabb;
    updated_pos.center.y += sign;

    // Check if the updated position would collide with anything
    for (auto [o_entity, o_psolid, o_aabb] : solids.each()) {
      bool same = entity == o_entity;

      if (!same && collide(updated_pos, o_aabb)) {
        Collision2D collision;
        const auto id_0 = static_cast<uint32_t>(entity);
        const auto id_1 = static_cast<uint32_t>(o_entity);
        collision.ent_id_0 = glm::min(id_0, id_1);
        collision.ent_id_1 = glm::max(id_0, id_1);
        collisions.push_back(collision);
      }
    }
    if (collisions.size() > 0)
      return collisions;

    // Move player if empty space
    aabb.center.y += sign;
    amount -= sign;
  }

  return collisions;
};

void
generate_broadphase_collisions_x(entt::registry& r, std::vector<Collision2D>& collisions)
{
  // store results of sorted aabb
  auto& physics = get_first_component<SINGLETON_PhysicsComponent>(r);
  physics.sorted_x.clear();

  // sort by axis
  const auto& sorted_x = r.group<PhysicsTransformXComponent>(entt::get<PhysicsTransformYComponent, AABB>);
  sorted_x.sort<PhysicsTransformXComponent>([](const auto& a, const auto& b) { return a.l < b.l; });

  std::vector<entt::entity> active_list;
  for (int i = 0; const auto& [new_obj, new_x, new_y, new_aabb] : sorted_x.each()) {

    physics.sorted_x.push_back(new_obj); // store sorted results

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
      const auto& [old_x, old_y, old_aabb] = sorted_x.get(old_obj);

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
};

void
generate_broadphase_collisions_y(entt::registry& r, std::vector<Collision2D>& collisions)
{
  // store results of sorted aabb
  auto& physics = get_first_component<SINGLETON_PhysicsComponent>(r);
  physics.sorted_y.clear();

  // sort by axis
  const auto& sorted_y = r.group<PhysicsTransformYComponent>(entt::get<PhysicsTransformXComponent, AABB>);
  sorted_y.sort<PhysicsTransformYComponent>([](const auto& a, const auto& b) { return a.t < b.t; });

  std::vector<entt::entity> active_list;
  for (int i = 0; const auto& [new_obj, new_y, new_x, new_aabb] : sorted_y.each()) {

    physics.sorted_y.push_back(new_obj); // store sorted results

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
      const auto& [old_y, old_x, old_aabb] = sorted_y.get(old_obj);

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
  generate_broadphase_collisions_x(r, collisions);
  generate_broadphase_collisions_y(r, collisions);

  // remove duplicates
  collisions.erase(std::unique(collisions.begin(), collisions.end()), collisions.end());
};

}; // namespace game2d