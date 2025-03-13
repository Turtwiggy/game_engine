#pragma once

#include <entt/fwd.hpp>

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

struct RemoveLerpWhenReachedTarget
{
  bool placeholder = true;
};

} // namespace game2d