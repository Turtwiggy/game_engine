#pragma once

namespace game2d {

struct SINGLE_ScreenshakeComponent
{
  float time = 0.1f; // how long to do it for
  float time_left = 0.0f;
};

struct RequestScreenshakeComponent
{
  bool placeholder = true;
};

} // namespace game2d