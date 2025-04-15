#pragma once

#include "modules/systems/system_cooldown/components.hpp"

namespace game2d {

struct OutOfBoundsTimer
{
  CooldownComponent cooldown_c; // time between damage ticks
};

} // namespace game2d