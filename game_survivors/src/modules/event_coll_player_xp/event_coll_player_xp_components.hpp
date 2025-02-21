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
  int xp_for_next_level = 10;
  int level = 1;
};

// Attached to a fixture to identify it
struct XpZoneComponent
{
  bool placeholder = true;
};

} // namespace game2d