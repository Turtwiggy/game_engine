#pragma once

#include <nlohmann/json.hpp>

// c++ headers
#include <cstdint>
#include <map>
#include <vector>

#include <entt/entt.hpp>

namespace game2d {

struct PhysicsTransformComponent
{
  uint32_t ent_id = 0;
  int x_tl = 0;
  int y_tl = 0;
  int w = 0;
  int h = 0;

  PhysicsTransformComponent() = default;
  PhysicsTransformComponent(int w, int h)
    : w(w)
    , h(h){};

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(PhysicsTransformComponent, w, h);
};

struct PhysicsSolidComponent
{
  bool placeholder = true;
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(PhysicsSolidComponent, placeholder);
};

struct PhysicsActorComponent
{
  bool placeholder = true;
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(PhysicsActorComponent, placeholder);
};

struct VelocityComponent
{
  int x = 0;
  int y = 0;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(VelocityComponent, x, y);
};

struct GridMoveComponent
{
  int x = 0;
  int y = 0;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(GridMoveComponent, x, y);
};

// A collision occurs between two entities
struct Collision2D
{
  uint32_t ent_id_0 = 0;
  uint32_t ent_id_1 = 0;
  bool collision_x = false;
  bool collision_y = false;
  bool dirty = false; // dirty means it occurred last frame
};

// events

struct WasCollidedWithComponent
{
  entt::entity instigator;
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