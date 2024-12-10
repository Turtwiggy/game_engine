#pragma once

namespace game2d {

struct CooldownComponent
{
  float time_max = 1.0f;
  float time = 0.0f;
};

} // namespace game2d