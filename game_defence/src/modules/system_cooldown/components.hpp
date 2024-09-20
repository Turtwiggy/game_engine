#pragma once

namespace game2d {

struct CooldownComponent
{
  float time = 1.0f;
  float time_left = 0.0f;
  bool on_cooldown = true;
};

} // namespace game2d