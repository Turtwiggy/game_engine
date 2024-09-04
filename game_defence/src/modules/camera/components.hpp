#pragma once

#include <entt/entt.hpp>

namespace game2d {

struct CameraFollow
{
  bool lerp_to_new_target = false;
};

struct CameraFreeMove
{
  bool placeholder = true;
};

struct CameraLerpToTarget
{
  bool placeholder = true;
};

} // namespace game2d