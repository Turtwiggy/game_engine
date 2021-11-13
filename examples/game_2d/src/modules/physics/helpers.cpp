// your header
#include "modules/physics/helpers.hpp"
#include "modules/renderer/components.hpp"

// engine headers
#include "engine/maths.hpp"

// c++ lib headers
#include <algorithm>
#include <iostream>
#include <vector>

namespace game2d {

bool
collide(const PhysicsObject& one, const PhysicsObject& two)
{
  // collision x-axis?
  bool collision_x = one.x_tl + one.w > two.x_tl && two.x_tl + two.w > one.x_tl;
  // collision y-axis?
  bool collision_y = one.y_tl + one.h > two.y_tl && two.y_tl + two.h > one.y_tl;

  return collision_x && collision_y;
}

bool
collides(const PhysicsObject& one, const std::vector<PhysicsObject>& others)
{
  for (const auto& two : others) {
    if (!two.collidable)
      continue;
    bool collides = collide(one, two);
    if (collides)
      return true;
  }
  return false;
}

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

}; // namespace game2d

void
game2d::generate_filtered_broadphase_collisions(const std::vector<PhysicsObject>& unsorted_aabb,
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

void
game2d::move_actors_dir(COLLISION_AXIS axis,
                        int& pos,
                        float& dx,
                        PhysicsObject& actor_aabb,
                        std::vector<PhysicsObject>& solids,
                        std::function<void()>& callback)
{
  constexpr auto Sign = [](int x) { return x == 0 ? 0 : (x > 0 ? 1 : -1); };

  int move_x = static_cast<int>(dx);
  if (move_x != 0) {
    dx -= move_x;
    int sign = Sign(move_x);
    PhysicsObject potential_aabb;
    potential_aabb.ent_id = actor_aabb.ent_id;
    while (move_x != 0) {

      potential_aabb.x_tl = actor_aabb.x_tl;
      if (axis == COLLISION_AXIS::X)
        potential_aabb.x_tl += sign;

      potential_aabb.y_tl = actor_aabb.y_tl;
      if (axis == COLLISION_AXIS::Y)
        potential_aabb.y_tl += sign;

      potential_aabb.w = actor_aabb.w;
      potential_aabb.h = actor_aabb.h;
      bool collision_with_solid = collides(potential_aabb, solids);
      if (collision_with_solid) // ah! collision, maybe actor-solid callback?
      {
        callback();
        // std::cout << "actor would collide X with solid if continue" << std::endl;
        break;
      }
      actor_aabb.x_tl = potential_aabb.x_tl;
      actor_aabb.y_tl = potential_aabb.y_tl;
      actor_aabb.w = potential_aabb.w;
      actor_aabb.h = potential_aabb.h;
      pos += sign;
      move_x -= sign;
    }
  }
};

void
game2d::print_solid(){
  // std::cout << "actor being squished between solids!" << std::endl;
};

void
game2d::print_actor(){
  // std::cout << "actor stopping itself; would collide with solid." << std::endl;
};