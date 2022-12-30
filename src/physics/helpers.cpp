// your header
#include "helpers.hpp"

// engine headers
#include "maths/maths.hpp"
#include "renderer/components.hpp"

namespace game2d {

glm::vec2
convert_tl_to_center(const PhysicsTransformComponent& po)
{
  glm::ivec2 half = glm::ivec2(int(po.w / 2.0f), int(po.h / 2.0f));
  return { po.x_tl + half.x, po.y_tl + half.y };
};

bool
collide(const PhysicsTransformComponent& one, const PhysicsTransformComponent& two)
{
  // collision x-axis?
  bool collision_x = one.x_tl + one.w > two.x_tl && two.x_tl + two.w > one.x_tl;
  // collision y-axis?
  bool collision_y = one.y_tl + one.h > two.y_tl && two.y_tl + two.h > one.y_tl;

  return collision_x && collision_y;
};

bool
collides(const PhysicsTransformComponent& one, const std::vector<PhysicsTransformComponent>& others)
{
  for (const auto& two : others) {
    bool collides = collide(one, two);
    if (collides)
      return true;
  }
  return false;
};

std::optional<Collision2D>
do_move(entt::registry& r,
        entt::entity& entity,
        int amount,
        TransformComponent& transform,
        const PhysicsTransformComponent& ptc,
        const CollisionAxis& axis)
{
  constexpr auto Sign = [](const int& x) { return x == 0 ? 0 : (x > 0 ? 1 : -1); };
  const auto& blocking_objects_view = r.view<const PhysicsSolidComponent, const PhysicsTransformComponent>();

  if (amount != 0) {
    if (axis == CollisionAxis::x)
      transform.position_dxdy.x -= amount;
    if (axis == CollisionAxis::y)
      transform.position_dxdy.y -= amount;
    int sign = Sign(amount);

    while (amount != 0) {

      // updated position
      PhysicsTransformComponent po;
      po.x_tl = ptc.x_tl;
      po.y_tl = ptc.y_tl;
      po.w = ptc.w;
      po.h = ptc.h;
      if (axis == CollisionAxis::x)
        po.x_tl += sign;
      if (axis == CollisionAxis::y)
        po.y_tl += sign;

      // Check if the updated position would collide with anything
      for (auto [o_entity, o_block, o_ptransform] : blocking_objects_view.each()) {
        bool same = entity == o_entity;
        if (!same && collide(po, o_ptransform)) {
          Collision2D collision;
          collision.ent_id_0 = static_cast<uint32_t>(entity);
          collision.ent_id_1 = static_cast<uint32_t>(o_entity);
          return collision;
        }
      }

      // Move player if empty space
      if (axis == CollisionAxis::x)
        transform.position.x += sign;
      if (axis == CollisionAxis::y)
        transform.position.y += sign;
      amount -= sign;
    }
  }
  return std::nullopt;
};

void
generate_broadphase_collisions(entt::registry& r,
                               const CollisionAxis& axis,
                               std::map<uint64_t, Collision2D>& collisions)
{
  // Sort by axis
  const auto& sorted_aabb = r.group<PhysicsTransformComponent, PhysicsActorComponent>();
  if (axis == CollisionAxis::x)
    sorted_aabb.sort<PhysicsTransformComponent>([&r](const auto& a, const auto& b) { return a.x_tl < b.x_tl; });
  else if (axis == CollisionAxis::y)
    sorted_aabb.sort<PhysicsTransformComponent>([&r](const auto& a, const auto& b) { return a.y_tl < b.y_tl; });

  std::vector<std::reference_wrapper<const PhysicsTransformComponent>> active_list;

  for (int i = 0; auto [entity, ptransform, pactor] : sorted_aabb.each()) {

    // begin on the left of sorted_aabb.
    // add the first item from sorted_aabb to active_list.
    if (i == 0) {
      active_list.push_back(ptransform);
      ++i;
      continue;
    }

    // have a look at the next item in axis_list,
    const auto& new_obj = ptransform;

    // compare it with all the items currently in active_list. (currently just 1)
    auto it_1 = active_list.begin();
    while (it_1 != active_list.end()) {
      const auto& old_obj = *it_1;

      int new_item_left = 0;
      int old_item_right = 0;

      if (axis == CollisionAxis::x) {
        // if the new item's left is > than the active_item's right
        new_item_left = new_obj.x_tl;
        old_item_right = old_obj.get().x_tl + old_obj.get().w;
      }
      if (axis == CollisionAxis::y) {
        // if the new item's top is > than the active_item's bottom
        new_item_left = new_obj.y_tl;
        old_item_right = old_obj.get().y_tl + old_obj.get().h;
      }

      if (new_item_left >= old_item_right) {
        // no possible collision!
        // remove the active_list item from the active list
        it_1 = active_list.erase(it_1);
      } else {
        // possible collision!
        // between new axis_list item and the current active_list item

        // Check existing collisions
        uint32_t old_ent_id = old_obj.get().ent_id;
        uint32_t new_ent_id = new_obj.ent_id;
        uint64_t unique_collision_id = engine::encode_cantor_pairing_function(old_ent_id, new_ent_id);

        Collision2D& coll = collisions[unique_collision_id];
        coll.ent_id_0 = old_ent_id;
        coll.ent_id_1 = new_ent_id;

        // update collision
        if (axis == CollisionAxis::x)
          coll.collision_x = true;
        if (axis == CollisionAxis::y)
          coll.collision_y = true;

        collisions[unique_collision_id] = coll;
        ++it_1;
      }
    }

    // Add the new item itself to active_list and continue with the next item in axis_list
    active_list.push_back(new_obj);

    ++i; // next
  }
};

void
generate_filtered_broadphase_collisions(entt::registry& r, std::map<uint64_t, Collision2D>& collision_results)
{
  collision_results.clear();

  // Do broad-phase check.
  std::map<uint64_t, Collision2D> collisions;
  generate_broadphase_collisions(r, CollisionAxis::x, collisions);
  generate_broadphase_collisions(r, CollisionAxis::y, collisions);

  // Use broad-phase results.
  for (const auto& coll : collisions) {
    const Collision2D& c = coll.second;
    if (c.collision_x && c.collision_y) {
      collision_results[coll.first] = coll.second;
    }
  }
};

}; // namespace game2d