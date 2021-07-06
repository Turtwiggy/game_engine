#pragma once

// other project headers
#include <functional>
#include <glm/glm.hpp>
#include <map>
#include <vector>

// your project headers
#include "2d_game_object.hpp"

namespace game2d {

enum class COLLISION_AXIS
{
  X,
  Y
};

struct Collision2D
{
  int ent_id_0;
  int ent_id_1;
  bool collision_x = false;
  bool collision_y = false;
  // CollisionLayer ent_0_layer;
  // CollisionLayer ent_1_layer;
};

struct CollisionEvent
{
  GameObject2D& go0;
  GameObject2D& go1;

  CollisionEvent(GameObject2D& go0, GameObject2D& go1)
    : go0(go0)
    , go1(go1){};
};

bool
game_collision_matrix(CollisionLayer& y_l1, CollisionLayer& x_l2);

void
generate_broadphase_collisions(const std::vector<std::reference_wrapper<GameObject2D>>& sorted_collidable_objects,
                               COLLISION_AXIS axis,
                               std::map<uint64_t, Collision2D>& collisions);

// broadphase: detect collisions that can actually happen and discard collisions which can't.
// sort and prune algorithm. note: suffers from large worlds with inactive objects.
// this issue can be solved by using multiple smaller SAP's which form a grid.
// note: i've adjusted this algortihm to do 2-axis SAP.
void
generate_filtered_broadphase_collisions(std::vector<std::reference_wrapper<GameObject2D>>& collidable,
                                        std::map<uint64_t, Collision2D>& filtered_collisions);

} // namespace game2d