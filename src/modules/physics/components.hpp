#pragma once

// c++ headers
#include <cstdint>
#include <map>
#include <vector>

namespace game2d {

// Note; for no collision, don't attach a CollidableComponent
enum class GameCollisionLayer
{
  ACTOR_PLAYER = 1,
  ACTOR_PLAYER_PET = 2,
  ACTOR_ASTEROID = 3,
  ACTOR_BULLET = 4,
  SOLID_WALL = 5,

  COUNT = 5,
};

struct PhysicsActorComponent
{
  GameCollisionLayer layer_id;
};

struct PhysicsSolidComponent
{
  GameCollisionLayer layer_id;
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

// specific for actor-actor collisions

// A collision occurs between two entities
struct Collision2D
{
  uint32_t ent_id_0 = 0;
  uint32_t ent_id_1 = 0;
  bool collision_x = false;
  bool collision_y = false;
  bool dirty = false; // dirty means it occurred last frame
};

// -- singleton components

struct SINGLETON_PhysicsComponent
{
  // resulting collisions from all registered objects
  std::map<uint64_t, Collision2D> frame_collisions;

  // persistent buffer, meaning the events enter, stay and exit can be tracked.
  std::map<uint64_t, Collision2D> persistent_collisions;

  // collision states
  std::vector<Collision2D> collision_enter;
  std::vector<Collision2D> collision_stay;
  std::vector<Collision2D> collision_exit;
};

} // game2d