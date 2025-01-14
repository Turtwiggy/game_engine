#pragma once

#include <entt/entt.hpp>

namespace game2d {

struct XpComponent
{
  bool placeholder = true;
};

// Monitoring the total number of xp collected
struct SINGLE_XpComponent
{
  int xp = 0;
  int xp_for_next_level = 20;
  int level = 1;
};

} // namespace game2d