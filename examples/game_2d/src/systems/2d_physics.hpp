#pragma once

// other project headers
#include <glm/glm.hpp>
#include <map>
#include <vector>

// your project headers
#include "game/2d_game_object.hpp"

namespace game2d {

enum class COLLISION_AXIS
{
  X,
  Y
};

struct Collision2D
{
  int ent_id_0;
  int ent_id_1;
  bool collision_x = false;
  bool collision_y = false;
};

bool
game_collision_matrix(CollisionLayer& y_l1, CollisionLayer& x_l2);

void
generate_broadphase_collisions(const std::vector<std::reference_wrapper<GameObject2D>>& sorted_collidable_objects,
                               COLLISION_AXIS axis,
                               std::map<uint64_t, Collision2D>& collisions);

} // namespace game2d