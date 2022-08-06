#pragma once

namespace game2d {

enum class TURN_PHASE
{
  MOVE,
  SHOOT,
  CHARGE,
  FIGHT,
  MORALE,

  END,
};

struct SINGLETON_TurnComponent
{
  int turn = 0;
  TURN_PHASE phase = TURN_PHASE::MOVE;
};

}; // namespace game2d