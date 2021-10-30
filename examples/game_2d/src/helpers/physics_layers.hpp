#pragma once

namespace game2d {

// Note; for no collision, don't attach a CollidableComponent
enum class GameCollisionLayer
{
  PLAYER = 1,
  GOAL = 2,
  WALL = 3,
  BALL = 4,
};

} // namespace game2d