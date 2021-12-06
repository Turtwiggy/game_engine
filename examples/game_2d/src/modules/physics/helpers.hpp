#pragma once

// components
#include "modules/physics/components.hpp"

// other lib headers
#include <glm/glm.hpp>

// other lib headers
#include <entt/entt.hpp>

// c++ lib headers
#include <functional>
#include <map>
#include <optional>
#include <vector>

namespace game2d {

// -- check object collisions

bool
collide(const PhysicsObject& one, const PhysicsObject& two);

// Checks collisions between an object and other objects
std::optional<CollisionInfo2D>
collides(const PhysicsObject& one, const std::vector<PhysicsObject>& others);

// Checks collisions between all objects
void
generate_filtered_broadphase_collisions(const std::vector<PhysicsObject>& unsorted_aabb,
                                        std::map<uint64_t, Collision2D>& collision_results);

// -- move objects

void
move_actors_dir(entt::registry& registry,
                COLLISION_AXIS axis,
                int& pos,
                float& dx,
                PhysicsObject& actor_aabb,
                std::vector<PhysicsObject>& solids,
                std::function<void(entt::registry&, CollisionInfo2D&)>& callback);

}; // namespace game2d
