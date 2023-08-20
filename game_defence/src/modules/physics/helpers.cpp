// your header
#include "helpers.hpp"

// engine headers
#include "entt/helpers.hpp"
#include "maths/maths.hpp"
#include "renderer/components.hpp"

#include <algorithm>

namespace game2d {

// glm::vec2
// convert_tl_to_center(const PhysicsTransformComponent& po)
// {
//   glm::ivec2 half = glm::ivec2(int(po.w / 2.0f), int(po.h / 2.0f));
//   return { po.x_tl + half.x, po.y_tl + half.y };
// };

// bool
// collides(const PhysicsTransformComponent& one, const std::vector<PhysicsTransformComponent>& others)
// {
//   for (const auto& two : others) {
//     bool collides = collide(one, two);
//     if (collides)
//       return true;
//   }
//   return false;
// };

bool
collide(const AABB& one, const AABB& two)
{
  const int one_w = one.x.r - one.x.l;
  const int one_h = one.y.t - one.y.b;
  const int two_w = two.x.r - two.x.l;
  const int two_h = two.y.t - two.y.b;

  // collision x-axis?
  bool collision_x = one.x.l + one_w > two.x.l && two.x.l + two_w > one.x.l;

  // collision y-axis?
  bool collision_y = one.y.t + one_h > two.y.t && two.y.t + two_h > one.y.t;

  return collision_x && collision_y;
};

std::vector<Collision2D>
do_move_x(entt::registry& r,
          entt::entity& entity,
          int amount,
          TransformComponent& transform,
          PhysicsTransformXComponent& x,
          const PhysicsTransformYComponent& y)
{
  std::vector<Collision2D> collisions;

  if (amount == 0)
    return collisions;

  const auto& solids =
    r.view<const PhysicsSolidComponent, const PhysicsTransformXComponent, const PhysicsTransformYComponent>();

  transform.position_dxdy.x -= amount;
  int sign = glm::sign(amount);

  while (amount != 0) {

    // updated position
    PhysicsTransformXComponent updated_pos;
    updated_pos.l = x.l + sign;
    updated_pos.r = x.r + sign;

    // Check if the updated position would collide with anything
    for (auto [o_entity, o_psolid, o_x, o_y] : solids.each()) {
      bool same = entity == o_entity;

      const AABB one{ x, y };
      const AABB two{ o_x, o_y };

      if (!same && collide(one, two)) {
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
    transform.position.x += sign;
    amount -= sign;

    // Update the physics transform component
    x.l = transform.position.x - glm::abs(transform.scale.x) / 2.0f;
    x.r = transform.position.x + glm::abs(transform.scale.x) / 2.0f;
  }

  return collisions;
};

std::vector<Collision2D>
do_move_y(entt::registry& r,
          entt::entity& entity,
          int amount,
          TransformComponent& transform,
          const PhysicsTransformXComponent& x,
          PhysicsTransformYComponent& y)
{
  std::vector<Collision2D> collisions;

  if (amount == 0)
    return collisions;

  const auto& solids =
    r.view<const PhysicsSolidComponent, const PhysicsTransformXComponent, const PhysicsTransformYComponent>();

  transform.position_dxdy.y -= amount;
  int sign = glm::sign(amount);

  while (amount != 0) {

    // updated position
    PhysicsTransformYComponent updated_pos;
    updated_pos.b = y.b + sign;
    updated_pos.t = y.t + sign;

    // Check if the updated position would collide with anything
    for (auto [o_entity, o_psolid, o_x, o_y] : solids.each()) {
      bool same = entity == o_entity;

      const AABB one{ x, y };
      const AABB two{ o_x, o_y };

      if (!same && collide(one, two)) {
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
    transform.position.y += sign;
    amount -= sign;

    // Update the physics transform component
    y.b = transform.position.y - glm::abs(transform.scale.y) / 2.0f;
    y.t = transform.position.y + glm::abs(transform.scale.y) / 2.0f;
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
  const auto& sorted_x = r.group<PhysicsTransformXComponent>(entt::get<PhysicsTransformYComponent>);
  sorted_x.sort<PhysicsTransformXComponent>([](const auto& a, const auto& b) { return a.l < b.l; });

  std::vector<entt::entity> active_list;
  for (int i = 0; const auto& [new_obj, new_x, new_y] : sorted_x.each()) {

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
      const auto& [old_x, old_y] = sorted_x.get(old_obj);

      // if the new item's left is > than the active_item's right
      if (new_x.l >= old_x.r) {
        // no possible collision!
        // remove the active_list item from the active list
        it_1 = active_list.erase(it_1);
      } else {
        // possible collision!
        // between new axis_list item and the current active_list item

        // Just do the AABB collision check
        const AABB one{ new_x, new_y };
        const AABB two{ old_x, old_y };
        if (collide(one, two)) {
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
  const auto& sorted_y = r.group<PhysicsTransformYComponent>(entt::get<PhysicsTransformXComponent>);
  sorted_y.sort<PhysicsTransformYComponent>([](const auto& a, const auto& b) { return a.t < b.t; });

  std::vector<entt::entity> active_list;
  for (int i = 0; const auto& [new_obj, new_y, new_x] : sorted_y.each()) {

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
      const auto& [old_y, old_x] = sorted_y.get(old_obj);

      // if the new item's top is > than the active_item's bottom
      if (new_y.b >= old_y.t) {
        // no possible collision!
        // remove the active_list item from the active list
        it_1 = active_list.erase(it_1);
      } else {
        // possible collision!
        // between new axis_list item and the current active_list item

        // Just do the AABB collision check
        const AABB one{ new_x, new_y };
        const AABB two{ old_x, old_y };
        if (collide(one, two)) {
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
  collisions.clear();
  generate_broadphase_collisions_x(r, collisions);
  generate_broadphase_collisions_y(r, collisions);

  // remove duplicates
  collisions.erase(std::unique(collisions.begin(), collisions.end()), collisions.end());
};

}; // namespace game2d