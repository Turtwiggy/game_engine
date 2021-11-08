#pragma once

// other lib headers
#include <glm/glm.hpp>

// c++ lib headers
#include <map>
#include <vector>

namespace game2d {

enum class PhysicsType
{
  SOLID,
  ACTOR,
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
  uint32_t ent_id_0 = true;
  uint32_t ent_id_1 = true;
  bool collision_x = false;
  bool collision_y = false;

  bool dirty = false; // dirty means it occured last frame
};

bool
collide(const PhysicsObject& one, const PhysicsObject& two);

// Checks collisions between an object and other objects
bool
collides(const PhysicsObject& one, const std::vector<PhysicsObject>& others);

// Checks collisions between all objects
void
generate_filtered_broadphase_collisions(const std::vector<PhysicsObject>& unsorted_aabb,
                                        std::map<uint64_t, Collision2D>& collision_results);

}; // namespace game2d
