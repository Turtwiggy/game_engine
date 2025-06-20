#pragma once

#include <glm/fwd.hpp>

namespace game2d {

struct SINGLE_ScreenshakeComponent
{
  float time = 0.1f; // how long to do it for
  float time_left = 0.0f;

  glm::vec2 strength{ 0, 0 };

  static SINGLE_ScreenshakeComponent instance;
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