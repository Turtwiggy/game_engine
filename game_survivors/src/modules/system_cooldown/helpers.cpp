#include "helpers.hpp"

namespace game2d {

void
reset_cooldown(CooldownComponent& c)
{
  c.time = c.time_max;
};

} // namespace game2d