#pragma once

#include <entt/fwd.hpp>

namespace game2d {

constexpr float ZOOM_IN = 0.25f;
constexpr float ZOOM_OUT = 1.75f;
static_assert((ZOOM_IN + ZOOM_OUT) * 0.5f == 1.0f);

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