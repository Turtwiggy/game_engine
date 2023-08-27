#pragma once

namespace game2d {

struct AttackCooldownComponent
{
  float time_between_attack = 0.10f;
  float time_between_attack_left = 0.0f;
  bool on_cooldown = true;
};

} // namespace game2d