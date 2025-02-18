#pragma once

namespace game2d {

struct SINGLE_ScreenshakeComponent
{
  float time = 0.1f; // how long to do it for
  float time_left = 0.0f;

  float strength = 0.0f;
};

enum class ScreenshakeType
{
  EXPLODE,
  SHOOT,
};

struct RequestScreenshakeComponent
{
  ScreenshakeType type;
};

} // namespace game2d