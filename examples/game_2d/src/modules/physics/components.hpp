#pragma once

// c++ headers
#include <map>
#include <vector>

namespace game2d {

// -- data

enum class COLLISION_AXIS
{
  X,
  Y
};

// A collision occurs between two entities
struct Collision2D
{
  uint32_t ent_id_0 = true;
  uint32_t ent_id_1 = true;
  bool collision_x = false;
  bool collision_y = false;
  bool dirty = false; // dirty means it occurred last frame
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
  // state
  bool collidable = true;
};

enum class PhysicsType
{
  SOLID,
  ACTOR,
};

// -- components

struct CollidableComponent
{
  uint32_t layer_id = 0;
  PhysicsType type = PhysicsType::ACTOR;
};

struct PhysicsSizeComponent
{
  int w = 0;
  int h = 0;

  PhysicsSizeComponent() = default;
  PhysicsSizeComponent(int w, int h)
    : w(w)
    , h(h){};
};

struct VelocityComponent
{
  float x = 0.0f;
  float y = 0.0f;
};

// -- singleton components

struct SINGLETON_PhysicsComponent
{
  // registered objects to use with physics system
  std::vector<PhysicsObject> collidable;

  // resulting collisions from all registered objects
  std::map<uint64_t, Collision2D> frame_collisions;

  // persistent buffer, meaning the events enter, stay and exit can be tracked.
  std::map<uint64_t, Collision2D> persistent_collisions;

  // collision states
  std::vector<Collision2D> collision_enter;
  std::vector<Collision2D> collision_stay;
  std::vector<Collision2D> collision_exit;
};

} // namespace game2d