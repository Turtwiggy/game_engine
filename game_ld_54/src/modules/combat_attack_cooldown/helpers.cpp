#include "helpers.hpp"

namespace game2d {

void
reset_cooldown(AttackCooldownComponent& c)
{
  c.time_between_attack_left = c.time_between_attack;
};

} // namespace game2d