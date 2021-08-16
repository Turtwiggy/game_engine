// your header
#include "2d_physics.hpp"

// engine headers
#include "engine/grid.hpp"
#include "engine/maths_core.hpp"

namespace game2d {

const int PHYSICS_GRID_SIZE = 100; // todo: for optimizing sweep and prune algorithm

bool
game_collision_matrix(CollisionLayer& y_l1, CollisionLayer& x_l2)
{
  int x_max = static_cast<int>(CollisionLayer::Count);
  int y = static_cast<int>(y_l1); // c1 is 0, 1, or 2
  int x = static_cast<int>(x_l2); // c2 is 0, 1, or 2

  // collision_matrix is a global (non-mutable) var
  bool val = grid::get_cell_mirrored_grid(GAME_COLL_MATRIX, x, y, x_max);
  return val;
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
        old_item_right = old_obj.get().pos.x + old_obj.get().physics_size.x;
      }
      if (axis == COLLISION_AXIS::Y) {
        // if the new item's top is > than the active_item's bottom
        new_item_left = new_obj.get().pos.y;
        old_item_right = old_obj.get().pos.y + old_obj.get().physics_size.y;
      }

      if (new_item_left > old_item_right) {
        // 2.3.1 Then remove the active_list item from the active list
        // as no possible collision!
        it_1 = active_list.erase(it_1);
      } else {
        // 2.3.2 possible collision!
        // between new axis_list item and the current active_list item

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

            // debug: collision is going to occur
            // if ((old_obj.get().collision_layer == CollisionLayer::Player &&
            //      new_obj.get().collision_layer == CollisionLayer::Obstacle) ||
            //     old_obj.get().collision_layer == CollisionLayer::Obstacle &&
            //       new_obj.get().collision_layer == CollisionLayer::Player) {
            //   std::cout << "pause!" << std::endl;
            // }

          } else {
            // new collision for these pairs!
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

void
generate_filtered_broadphase_collisions(std::vector<std::reference_wrapper<GameObject2D>>& collidable,
                                        std::map<uint64_t, Collision2D>& filtered_collisions)
{
  // Assign grid postions to all entities
  // pre-physics: update grid position
  for (auto& e : collidable) {
    grid::get_unique_cells(e.get().pos, e.get().physics_size, PHYSICS_GRID_SIZE, e.get().in_physics_grid_cell);
  }

  // Do broad-phase check.
  std::map<uint64_t, Collision2D> collisions;

  // Sort entities by X-axis
  std::vector<std::reference_wrapper<GameObject2D>> sorted_collidable_x = collidable;
  std::sort(sorted_collidable_x.begin(),
            sorted_collidable_x.end(),
            [&](std::reference_wrapper<GameObject2D>& a, std::reference_wrapper<GameObject2D>& b) {
              return a.get().pos.x < b.get().pos.x;
            });
  // SAP x-axis
  std::vector<std::pair<int, int>> potential_collisions_x;
  generate_broadphase_collisions(sorted_collidable_x, COLLISION_AXIS::X, collisions);

  // Sort entities by Y-axis
  std::vector<std::reference_wrapper<GameObject2D>> sorted_collidable_y = collidable;
  std::sort(sorted_collidable_y.begin(),
            sorted_collidable_y.end(),
            [&](std::reference_wrapper<GameObject2D>& a, std::reference_wrapper<GameObject2D>& b) {
              return a.get().pos.y < b.get().pos.y;
            });
  // SAP y-axis
  std::vector<std::pair<int, int>> potential_collisions_y;
  generate_broadphase_collisions(sorted_collidable_y, COLLISION_AXIS::Y, collisions);

  // use broad-phase results....
  for (auto& coll : collisions) {
    Collision2D c = coll.second;
    if (c.collision_x && c.collision_y) {
      filtered_collisions[coll.first] = coll.second;
    }
  }
};

}