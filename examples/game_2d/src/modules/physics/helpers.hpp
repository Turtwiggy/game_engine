#pragma once

// other lib headers
#include <glm/glm.hpp>

// c++ lib headers
#include <map>
#include <vector>

namespace game2d {

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

// A representation of a line, where only 1 axis matters
struct Line
{
  int point_a;
  int point_b;
}

// A collision occurs between two entities
struct Collision2D
{
  uint32_t ent_id_0 = true;
  uint32_t ent_id_1 = true;
  bool collision_x = false;
  bool collision_y = false;

  bool dirty = false; // dirty means it occured last frame
};

// broadphase: detect collisions that can actually happen and discard collisions which can't.
// sort and prune algorithm. note: suffers from large worlds with inactive objects.
// this issue can be solved by using multiple smaller SAP's which form a grid.
// note: i've adjusted this algortihm to do 2-axis SAP.
void
generate_filtered_broadphase_collisions(const std::vector<PhysicsObject>& unsorted_aabb,
                                        std::map<uint64_t, Collision2D>& collision_results);

}; // namespace game2d
