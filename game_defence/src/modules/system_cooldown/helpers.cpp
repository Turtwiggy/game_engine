#include "helpers.hpp"

namespace game2d {

void
reset_cooldown(CooldownComponent& c)
{
  c.on_cooldown = true;
  c.time_left = c.time;
};

} // namespace game2d