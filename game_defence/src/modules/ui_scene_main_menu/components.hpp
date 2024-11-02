#pragma once

#include <string>
#include <vector>

namespace game2d {

struct SINGLE_MainMenuUI
{
  std::vector<std::string> hovered_buttons;
};

struct MenuToNextSceneInfo
{
  int level = 0;

  bool processed = false;
};

} // namespace game2d