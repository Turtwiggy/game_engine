#pragma once

namespace game2d {

struct GameOverComponent
{
  bool win_condition = false;
  std::string reason = "Empty";
};

} // namespace game2d