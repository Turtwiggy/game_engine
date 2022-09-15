#pragma once

// components
#include "modules/physics/components.hpp"
#include "modules/renderer/components.hpp"

// other lib headers
#include <glm/glm.hpp>

// other lib headers
#include <entt/entt.hpp>

// c++ lib headers
#include <map>
#include <vector>

namespace game2d {

enum class CollisionAxis
{
  x,
  y
};

struct PhysicsObject
{
  uint32_t ent_id = 0;
  // aabb
  int x_tl = 0;
  int y_tl = 0;
  int w = 0;
  int h = 0;
  // state
  bool collidable = true;
};

// Checks collisions between actor-actors.
void
generate_filtered_broadphase_collisions(const std::vector<PhysicsObject>& unsorted_aabb,
                                        std::map<uint64_t, Collision2D>& collision_results);

void
get_solids_as_physics_objects(entt::registry& registry, std::vector<PhysicsObject>& result);

void
get_actors_as_physics_objects(entt::registry& registry, std::vector<PhysicsObject>& result);

bool
collides(const PhysicsObject& one, const std::vector<PhysicsObject>& others);

void
do_move_x(TransformComponent& t, std::vector<PhysicsObject>& solids, int& amount);

void
do_move_y(TransformComponent& t, std::vector<PhysicsObject>& solids, int& amount);

}; // namespace game2d
