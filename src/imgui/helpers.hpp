#pragma once

#include <string>
#include <vector>

namespace game2d {

struct WomboComboIn
{
  std::string label;

  int current_index = 0;
  const std::vector<std::string>& items;

  WomboComboIn(const std::vector<std::string>& i)
    : items(i){};
};
struct WomboComboOut
{
  int selected = 0;
};

WomboComboOut
draw_wombo_combo(const WomboComboIn& in);

} // namespace game2d