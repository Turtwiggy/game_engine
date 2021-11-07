#pragma once

namespace game2d {

// Note; for no collision, don't attach a CollidableComponent
enum class GameCollisionLayer
{
  ACTOR_PLAYER = 1,
  ACTOR_GOAL = 2,
  ACTOR_BALL = 3,
  SOLID_WALL = 4,
};

} // namespace game2d