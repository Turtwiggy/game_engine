// your header
#include "helpers.hpp"

// engine headers
#include "engine/maths/maths.hpp"
#include "modules/renderer/components.hpp"

// c++ lib headers
#include <algorithm>
#include <iostream>

namespace game2d {

void
get_solids_as_physics_objects(entt::registry& registry, std::vector<PhysicsObject>& result)
{
  {
    const auto& entt_solids =
      registry.view<const TransformComponent, const PhysicsSizeComponent, const PhysicsSolidComponent>();

    PhysicsObject po;
    entt_solids.each([&result, &po](const auto entity,
                                    const TransformComponent& transform,
                                    const PhysicsSizeComponent& size,
                                    const PhysicsSolidComponent& solid) {
      po.ent_id = static_cast<uint32_t>(entity);
      po.x_tl = static_cast<int>(transform.position.x - glm::abs(size.w) / 2.0f);
      po.y_tl = static_cast<int>(transform.position.y - glm::abs(size.h) / 2.0f);
      po.w = glm::abs(size.w);
      po.h = glm::abs(size.h);
      result.push_back(po);
    });
  }
}

void
get_actors_as_physics_objects(entt::registry& registry, std::vector<PhysicsObject>& result)
{
  {
    const auto& entt_actors =
      registry.view<const TransformComponent, const PhysicsSizeComponent, const PhysicsActorComponent>();
    PhysicsObject po;
    entt_actors.each([&result, &po](const auto entity,
                                    const TransformComponent& transform,
                                    const PhysicsSizeComponent& size,
                                    const PhysicsActorComponent& actor) {
      po.ent_id = static_cast<uint32_t>(entity);
      po.x_tl = static_cast<int>(transform.position.x - glm::abs(size.w) / 2.0f);
      po.y_tl = static_cast<int>(transform.position.y - glm::abs(size.h) / 2.0f);
      po.w = glm::abs(size.w);
      po.h = glm::abs(size.h);
      result.push_back(po);
    });
  }
}

glm::vec2
convert_tl_to_center(const PhysicsObject& po)
{
  glm::ivec2 half = glm::ivec2(int(po.w / 2.0f), int(po.h / 2.0f));
  return { po.x_tl + half.x, po.y_tl + half.y };
};

bool
collide(const PhysicsObject& one, const PhysicsObject& two)
{
  // collision x-axis?
  bool collision_x = one.x_tl + one.w > two.x_tl && two.x_tl + two.w > one.x_tl;
  // collision y-axis?
  bool collision_y = one.y_tl + one.h > two.y_tl && two.y_tl + two.h > one.y_tl;

  return collision_x && collision_y;
};

void
generate_broadphase_collisions(const std::vector<std::reference_wrapper<const PhysicsObject>>& sorted_aabb,
                               COLLISION_AXIS axis,
                               std::map<uint64_t, Collision2D>& collisions)
{
  std::vector<std::reference_wrapper<const PhysicsObject>> active_list;

  // begin on the left of sorted_aabb.
  // add the first item from sorted_aabb to active_list.
  if (sorted_aabb.size() > 0) {
    active_list.push_back(sorted_aabb[0]);
  }

  for (int i = 1; i < sorted_aabb.size(); i++) {
    const auto& new_obj = sorted_aabb[i];

    // have a look at the next item in axis_list,
    // and compare it with all the items currently in active_list. (currently just 1)
    std::vector<std::reference_wrapper<const PhysicsObject>>::iterator it_1 = active_list.begin();
    while (it_1 != active_list.end()) {
      const auto& old_obj = *it_1;

      int new_item_left = 0;
      int old_item_right = 0;

      if (axis == COLLISION_AXIS::X) {
        // if the new item's left is > than the active_item's right
        new_item_left = new_obj.get().x_tl;
        old_item_right = old_obj.get().x_tl + old_obj.get().w;
      }
      if (axis == COLLISION_AXIS::Y) {
        // if the new item's top is > than the active_item's bottom
        new_item_left = new_obj.get().y_tl;
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
        uint64_t unique_collision_id =
          engine::encode_cantor_pairing_function(old_obj.get().ent_id, new_obj.get().ent_id);

        Collision2D& coll = collisions[unique_collision_id];
        coll.ent_id_0 = old_obj.get().ent_id;
        coll.ent_id_1 = new_obj.get().ent_id;

        // update collision
        if (axis == COLLISION_AXIS::X)
          coll.collision_x = true;
        if (axis == COLLISION_AXIS::Y)
          coll.collision_y = true;

        collisions[unique_collision_id] = coll;
        ++it_1;
      }
    }

    // Add the new item itself to active_list and continue with the next item in axis_list
    active_list.push_back(new_obj);
  }
};

void
generate_filtered_broadphase_collisions(const std::vector<PhysicsObject>& unsorted_aabb,
                                        std::map<uint64_t, Collision2D>& collision_results)
{
  // Do broad-phase check.

  // entities sorted by X-axis
  std::vector<std::reference_wrapper<const PhysicsObject>> sorted_collidable_x(unsorted_aabb.begin(),
                                                                               unsorted_aabb.end());
  std::sort(sorted_collidable_x.begin(),
            sorted_collidable_x.end(),
            [&](std::reference_wrapper<const PhysicsObject>& a, std::reference_wrapper<const PhysicsObject>& b) {
              return a.get().x_tl < b.get().x_tl;
            });

  // entities sorted by Y-axis
  std::vector<std::reference_wrapper<const PhysicsObject>> sorted_collidable_y(unsorted_aabb.begin(),
                                                                               unsorted_aabb.end());
  std::sort(sorted_collidable_y.begin(),
            sorted_collidable_y.end(),
            [&](std::reference_wrapper<const PhysicsObject>& a, std::reference_wrapper<const PhysicsObject>& b) {
              return a.get().y_tl < b.get().y_tl;
            });

  std::map<uint64_t, Collision2D> collisions;
  generate_broadphase_collisions(sorted_collidable_x, COLLISION_AXIS::X, collisions);
  generate_broadphase_collisions(sorted_collidable_y, COLLISION_AXIS::Y, collisions);

  // use broad-phase results....
  collision_results.clear();
  for (const auto& coll : collisions) {
    const Collision2D& c = coll.second;
    if (c.collision_x && c.collision_y) {
      collision_results[coll.first] = coll.second;
    }
  }
};

bool
collides(const PhysicsObject& one, const std::vector<PhysicsObject>& others)
{
  for (const auto& two : others) {
    if (!two.collidable)
      continue;
    if (one.ent_id == two.ent_id)
      continue;
    bool collides = collide(one, two);
    // note, doesn't return "others" ids, stops when any collision
    if (collides) {
      return true;
    }
  }
  return false;
};

}; // namespace game2d