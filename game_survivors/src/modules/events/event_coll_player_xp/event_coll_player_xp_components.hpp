#pragma once

#include <entt/fwd.hpp>

namespace game2d {

struct XpComponent
{
  bool levelup = false;
};

// Monitoring the total number of xp collected
struct SINGLE_XpComponent
{
#if defined(_DEBUG)
  int xp = 0;
#else
  int xp = 0;
#endif
  int xp_for_next_level = 15;
  int level = 1;
};

// Attached to a fixture to identify it
struct XpZoneComponent
{
  bool placeholder = true;
};

} // namespace game2d