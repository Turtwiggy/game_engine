#pragma once

#include "components.hpp"

// other lib headers
#include <glm/glm.hpp>

// other lib headers
#include <entt/entt.hpp>

// c++ lib headers
#include <vector>

namespace game2d {

bool
collide(const AABB& one, const AABB& two);

bool
collide(const CircleCollider& a, const CircleCollider& b);

template<class T>
bool
contains(const AABB& a, const T& point)
{
  const glm::ivec2 a_tl_pos = { a.center.x - a.size.x / 2, a.center.y - a.size.y / 2 };
  const bool in_x = point.x >= a_tl_pos.x && point.x <= a_tl_pos.x + a.size.x;
  const bool in_y = point.y >= a_tl_pos.y && point.y <= a_tl_pos.y + a.size.y;
  return in_x && in_y;
};

// Checks collisions between actor-actors.
void
generate_filtered_broadphase_collisions(entt::registry& r, std::vector<Collision2D>& results);

}; // namespace game2d
