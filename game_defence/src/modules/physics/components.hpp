#pragma once

#include <nlohmann/json.hpp>

// c++ headers
#include <vector>

#include <entt/entt.hpp>
#include <glm/glm.hpp>

namespace game2d {

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

struct AABB
{
  PhysicsTransformXComponent x;
  PhysicsTransformYComponent y;
  // note: could calculate from e.g. x.r-x.l but
  // int inaccuracies mean value can be slightly off
  glm::ivec2 center;
  glm::ivec2 size;
};

struct PhysicsSolidComponent
{
  bool placeholder = true;
  NLOHMANN_DEFINE_TYPE_INTRUSIVE(PhysicsSolidComponent, placeholder);
};

struct PhysicsActorComponent
{
  bool placeholder = true;
};

struct VelocityComponent
{
  int x = 0;
  int y = 0;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(VelocityComponent, x, y);
};

struct GridMoveComponent
{
  float x = 0.0f;
  float y = 0.0f;

  NLOHMANN_DEFINE_TYPE_INTRUSIVE(GridMoveComponent, x, y);
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