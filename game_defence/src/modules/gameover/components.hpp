#pragma once

#include <string>

namespace game2d {

struct SINGLETON_GameOver
{
  bool game_is_over = false;
  std::string reason;

  bool win_condition = false;

  // activated_gameover: used by the gameover system. do not set
  bool activated_gameover = false;
};

};