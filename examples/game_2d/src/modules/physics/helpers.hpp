#pragma once

// c++ lib headers
#include <map>
#include <vector>

namespace game2d {

// A unique collision layer
struct CollisionLayer
{
  int id;

  CollisionLayer(int id)
    : id(id){};
};

// A physics object needs an entity id, and size info
struct PhysicsObject
{
  uint32_t ent_id;
  // aabb
  int x_tl;
  int y_tl;
  int w;
  int h;
};

// A collision occurs between two entities
struct Collision2D
{
  uint32_t ent_id_0;
  uint32_t ent_id_1;
  bool collision_x;
  bool collision_y;
};

// broadphase: detect collisions that can actually happen and discard collisions which can't.
// sort and prune algorithm. note: suffers from large worlds with inactive objects.
// this issue can be solved by using multiple smaller SAP's which form a grid.
// note: i've adjusted this algortihm to do 2-axis SAP.
void
generate_filtered_broadphase_collisions(const std::vector<PhysicsObject>& unsorted_aabb,
                                        std::map<uint64_t, Collision2D>& collision_results);

}; // namespace game2d
