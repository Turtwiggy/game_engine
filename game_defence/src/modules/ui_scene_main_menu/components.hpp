#pragma once

#include <string>
#include <vector>

namespace game2d {

struct SINGLE_MainMenuUI
{
  int level = 1;

  // probably shouldnt be here
  std::vector<std::string> random_names;
};

} // namespace game2d