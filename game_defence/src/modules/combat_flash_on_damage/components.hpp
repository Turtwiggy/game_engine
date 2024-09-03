#pragma once

namespace game2d {

struct RequestFlashComponent
{
  int milliseconds_left = 0.25 * 1000;
  bool flashing = false;
};

} // namespace game2d