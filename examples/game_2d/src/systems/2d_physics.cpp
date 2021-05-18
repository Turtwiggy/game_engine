// your header
#include "systems/2d_physics.hpp"

// engine headers
#include "engine/maths_core.hpp"

namespace game2d {

int
get_layer_value(int i, int n)
{
  if (i <= 0)
    return 0;
  return get_layer_value(i - 1, n) + (n - (i - 1));
}

bool
game_collision_matrix(CollisionLayer& y_l1, CollisionLayer& x_l2)
{
  int c1 = static_cast<int>(y_l1); // c1 is 0, 1, or 2
  int c2 = static_cast<int>(x_l2); // c2 is 0, 1, or 2
  if (c2 < c1) {
    // make c1 lower value
    int temp = c1;
    c1 = c2;
    c2 = temp;
  }

  int x_max = static_cast<int>(CollisionLayer::Count);
  // int val = c1 * x_max + c2;
  int val = get_layer_value(c1, x_max) - c1 + c2;

  static const std::vector<bool> collision_matrix = {
    true, // default_default_0_0      // 0
    true, // default_player_0_1       // 1
    true, // default_bullet_0_2       // 2
    true, // default_detroyable 0_3   // 3

    true,  // player_player_1_1       // 4
    false, // player_bullet_1_2       // 5
    true,  // player_destroyable_1_3  // 6

    false, // bullet_bullet_2_2       // 7
    true,  // bullet_destroyable_2_3  // 8

    false, // destroyable_destroyable_3_3 // 9
  };

  return collision_matrix[val];
}

void
generate_broadphase_collisions(const std::vector<std::reference_wrapper<GameObject2D>>& sorted_collidable_objects,
                               COLLISION_AXIS axis,
                               std::map<uint64_t, Collision2D>& collisions)
{

  // 2. begin on the left of above list.
  std::vector<std::reference_wrapper<GameObject2D>> active_list;

  // 2.1 add the first item from axis_list to active_list.
  if (sorted_collidable_objects.size() > 0) {
    active_list.push_back(sorted_collidable_objects[0]);
  }

  for (int i = 1; i < sorted_collidable_objects.size(); i++) {

    const std::reference_wrapper<GameObject2D>& new_obj = sorted_collidable_objects[i];

    // 2.2 have a look at the next item in axis_list,
    // and compare it with all the items currently in active_list. (currently just 1)
    std::vector<std::reference_wrapper<GameObject2D>>::iterator it_1 = active_list.begin();
    while (it_1 != active_list.end()) {

      std::reference_wrapper<GameObject2D>& old_obj = *it_1;

      float new_item_left = 0;
      float old_item_right = 0;
      if (axis == COLLISION_AXIS::X) {
        // if the new item's left is > than the active_item's right
        new_item_left = new_obj.get().pos.x;
        old_item_right = old_obj.get().pos.x + old_obj.get().size.x;
      }
      if (axis == COLLISION_AXIS::Y) {
        // if the new item's top is > than the active_item's bottom
        new_item_left = new_obj.get().pos.y;
        old_item_right = old_obj.get().pos.y + old_obj.get().size.y;
      }

      if (new_item_left > old_item_right) {
        // 2.3.1 Then remove the active_list item from the active list
        // as no possible collision!
        it_1 = active_list.erase(it_1);
      } else {
        // 2.3.2 possible collision!
        // // between new axis_list item and the current active_list item
        // std::pair<std::reference_wrapper<GameObject2D>, std::reference_wrapper<GameObject2D>> collision =
        //   std::pair<std::reference_wrapper<GameObject2D>, std::reference_wrapper<GameObject2D>>(new_obj, old_obj);

        // Check game logic!
        bool valid_collision = game_collision_matrix(new_obj.get().collision_layer, old_obj.get().collision_layer);
        if (valid_collision) {

          // Check existing collisions
          uint64_t unique_collision_id =
            fightingengine::encode_cantor_pairing_function(old_obj.get().id, new_obj.get().id);
          if (collisions.find(unique_collision_id) != collisions.end()) {
            // collision for these pairs!
            Collision2D& coll = collisions[unique_collision_id];
            if (axis == COLLISION_AXIS::X)
              coll.collision_x = true;
            if (axis == COLLISION_AXIS::Y)
              coll.collision_y = true;

            collisions[unique_collision_id] = coll;
          } else {
            // new collision
            // collision for these pairs!
            Collision2D& coll = collisions[unique_collision_id];
            coll.ent_id_0 = old_obj.get().id;
            coll.ent_id_1 = new_obj.get().id;
            if (axis == COLLISION_AXIS::X)
              coll.collision_x = true;
            if (axis == COLLISION_AXIS::Y)
              coll.collision_y = true;

            collisions[unique_collision_id] = coll;
          }
        }

        ++it_1;
      }
    }

    // 2.4 Add the new item itself to active_list and continue with the next item in axis_list
    active_list.push_back(new_obj);
  }
}

}