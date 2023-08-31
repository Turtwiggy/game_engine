#pragma once

namespace game2d {

struct FlashOnDamageComponent
{
  bool started = false;
  int milliseconds_left = 200;
};

} // namespace game2d