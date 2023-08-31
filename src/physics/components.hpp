#pragma once

#include <nlohmann/json.hpp>

// c++ headers
#include <vector>

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace game2d {

struct AABB
{
  glm::ivec2 center{ 0, 0 };
  glm::ivec2 size{ 0, 0 }; // width and height
  // entt::entity debug_aabb = entt::null;
};

struct PhysicsTransformXComponent
{
  int l = 0;
  int r = 0;
};

struct PhysicsTransformYComponent
{
  int t = 0;
  int b = 0;
};

struct PhysicsSolidComponent
{
  bool placeholder = true;
};

struct PhysicsActorComponent
{
  bool placeholder = true;
};

struct VelocityComponent
{
  float x = 0.0f;
  float y = 0.0f;
  float remainder_x = 0.0f;
  float remainder_y = 0.0f;

  float base_speed = 100.0f; // influences velocity
};

// A collision occurs between two entities
struct Collision2D
{
  uint32_t ent_id_0 = 0;
  uint32_t ent_id_1 = 0;
  bool dirty = false; // dirty means it occurred last frame

  // spaceship operator
  auto operator<=>(const Collision2D&) const = default;
};

// -- singleton components

struct SINGLETON_PhysicsComponent
{
  // resulting collisions from all registered objects
  std::vector<Collision2D> frame_collisions;

  // persistent buffer, meaning the events enter, stay and exit can be tracked.
  std::vector<Collision2D> persistent_collisions;

  // collision states
  std::vector<Collision2D> collision_enter;
  std::vector<Collision2D> collision_stay;
  std::vector<Collision2D> collision_exit;

  std::vector<entt::entity> sorted_x;
  std::vector<entt::entity> sorted_y;
};

} // game2d