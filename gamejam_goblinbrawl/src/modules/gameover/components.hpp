#pragma once

#include <string>

namespace game2d {

struct RequestGameOver
{
  bool win_condition = true;
  std::string reason = "Default";
};

};