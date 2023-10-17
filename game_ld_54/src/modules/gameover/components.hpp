#pragma once

#include <string>

namespace game2d {

struct SINGLETON_GameOver
{
  bool game_is_over = false;
  std::string reason;
};

};