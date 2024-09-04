#pragma once

namespace game2d {

struct RequestFlashComponent
{
  int milliseconds_left = static_cast<int>(0.25f * 1000);
  bool flashing = false;
};

} // namespace game2d|